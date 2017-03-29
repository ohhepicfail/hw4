#include "interpreter.h"
#include <list>
#include <string>
#include <iostream>

namespace ipr {

    double Interpreter::run () {
        calculate ();

        auto find_res = var_value_.find ("result");
        if (find_res == var_value_.end ()) {
            printf ("No variable 'result'\n");
            abort ();
        }
        return find_res->second;
    }


    void Interpreter::calculate () {
        struct scope_data {
            scope_data (decltype (var_value_) hash_table, decltype (parser_.get_next ()) var_list) : 
                                                        htable_ (hash_table)
                                                      , var_list_ (var_list) {} 
            decltype (var_value_) htable_;
            decltype (parser_.get_next ()) var_list_;
        };
        enum Label {
            IF_B,
            IF_C,
            WHILE_B,
            WHILE_C,
            ASSIGN_B,  
            ENDIF_B,
            ENDWHILE_B,
            ENDWHILE_C,
            ENDFUNC_B
        };
        struct function_data {
            std::remove_reference<decltype (parser_.get_next_expr ())>::type expr_;
            std::stack<double> helper_;
            decltype (var_value_) htable_;
            Label upper_level_;
            function_data (decltype (expr_) expr
                         , decltype (helper_) intermediate
                         , decltype (htable_) htable
                         , decltype (upper_level_) up_l) :
                        expr_ (expr), 
                        helper_ (intermediate),
                        htable_ (htable),
                        upper_level_ (up_l) {}
        };
        
        std::stack <scope_data> scope;
        std::stack<function_data> func_data;
        auto cur_node = parser_.get_next ();
    
        auto call_func = [&func_data, this] (decltype (function_data::expr_)& expr
                                           , decltype (function_data::helper_)& intermediate
                                           , decltype (function_data::upper_level_) up_l) {
            parser_.load_func (expr.top ());
            expr.pop ();
            func_data.push (function_data (expr, intermediate, var_value_, up_l));
            
            auto cur_node = parser_.get_next ();
            create_htable (cur_node);
        };

        bool from_func = false; 
        std::remove_reference<decltype (parser_.get_next_expr ())>::type expr;
        std::stack<double> intermediate_st; 

        auto cleanup = [&expr, &intermediate_st] {
            decltype (expr) ().swap (expr);
            decltype (intermediate_st) ().swap (intermediate_st);
        };

        auto print = [] (std::remove_reference<decltype (parser_.get_next_expr ())>::type expr) {
            std::cout << "printing stack\n";
            while (!expr.empty ()) {
                auto tmp = expr.top ();
                switch (tmp->get_type ()) {
                    default: assert (0); break;
                    case type::VAL: std::cout << "val " << tmp->get_val () << std::endl; break;
                    case type::VAR: std::cout << "var " << tmp->get_var () << std::endl; break;
                    case type::OP : std::cout << "op " << op::string_eq (tmp->get_op ()) <<std::endl; break;
                }
                expr.pop ();
            }
            std::cout << "stack was printed\n";
        };

        Label cur_label;

        for (;;) {
            if (cur_node->get_type () != type::OP) {
                printf ("Statement has no effect\n");
                abort ();
            }
            if (!from_func) {
                switch (cur_node->get_op ()) {
                    default: assert (0); break;
                    case op::WHILE:     cur_label = WHILE_B; break;
                    case op::IF:        cur_label = IF_B; break;
                    case op::ENDIF:     cur_label = ENDIF_B; break;
                    case op::ENDWHILE:  cur_label = ENDWHILE_B; break;
                    case op::ENDFUNC:   cur_label = ENDFUNC_B; break;
                    case op::ASSIGN:    cur_label = ASSIGN_B; break;
                }
            }
            switch (cur_label) {
                default:
                    assert (0);
                break;

                case WHILE_B:
                case IF_B:
                    cleanup ();
                    expr = parser_.get_next_expr ();
                case IF_C:
                case WHILE_C: {
                    auto completed = calculate_expr (expr, intermediate_st);
                    if (!completed) {
                        print (expr);
                        auto unit = expr.top ();
                        assert (unit->get_type () == type::OP && unit->get_op () == op::CALL);
                        call_func (expr, intermediate_st, cur_label == IF_B || cur_label == IF_C ? IF_C : WHILE_C);
                        break;
                    }
                    if (intermediate_st.top ()) {
                        cur_node = parser_.get_next ();
                        scope.push (scope_data (var_value_, cur_node));
                        create_htable (cur_node);                
                    }
                    else
                        parser_.skip ();
                } break;

                case ENDIF_B: {
                    if (scope.empty ()) {
                        printf ("without previous if\n");
                        abort ();
                    }
                    update_htable (scope.top ().var_list_, scope.top ().htable_);
                    scope.pop ();
                } break;

                case ENDWHILE_B:
                    if (scope.empty ()) {
                        printf ("without previous while\n");
                        abort ();
                    }

                    parser_.repeat ();
                    cleanup ();
                    expr = parser_.get_next_expr ();
                case ENDWHILE_C: {
                    auto completed = calculate_expr (expr, intermediate_st);
                    if (!completed) {
                        auto unit = expr.top ();
                        assert (unit->get_type () == type::OP && unit->get_op () == op::CALL);
                        call_func (expr, intermediate_st, ENDWHILE_C);
                        break;
                    }

                    if (!intermediate_st.top ()) {
                        parser_.skip ();
                        update_htable (scope.top ().var_list_, scope.top ().htable_);
                        scope.pop ();
                    }
                    else
                        parser_.get_next ();    // skip capture
                    
                } break;

                case ENDFUNC_B: {
                    from_func = true;

                    auto find_res = var_value_.find ("result");
                    if (find_res == var_value_.end ()) {
                        printf ("No variable 'result'\n");
                        abort ();
                    }
                    std::cout << "func result " << find_res->second << std::endl;
                    expr = func_data.top ().expr_;
                    print (expr);
                    intermediate_st = func_data.top ().helper_;
                    var_value_ = func_data.top ().htable_;
                    cur_label = func_data.top ().upper_level_;
                    func_data.pop ();
                    std::cout << expr.empty () << std::endl;
                    intermediate_st.push (find_res->second);
                continue;
                } break;

                case ASSIGN_B: {
                    auto var = parser_.get_next ();
                    assert (var);
                    assert (var->get_type () == type::VAR); 

                    cleanup ();
                    expr = parser_.get_next_expr ();
                    auto completed = calculate_expr (expr, intermediate_st);
                    assert (completed);
                    auto res = intermediate_st.top ();
                    auto find_res = var_value_.find (var->get_var ());
                    if (find_res == var_value_.end ())
                        var_value_.insert (std::make_pair (var->get_var (), res));
                    else
                        find_res->second = res;
                    
                } break;
            }
            
            from_func = false;
            cur_node = parser_.get_next ();
            if (!cur_node)
                break;
        }

    }


    bool Interpreter::calculate_expr (decltype(parser_.get_next_expr ())& expr, std::stack<double>& intermediate_st) {
        while (!expr.empty ()) {
            auto unit = expr.top ();
            switch (unit->get_type ()) {
                default: 
                    assert (0);
                break;

                case type::VAL:
                    intermediate_st.push (unit->get_val ());
                break;

                case type::VAR: {
                    auto find_res = var_value_.find (unit->get_var ());
                    if (find_res == var_value_.end ()) {
                        printf ("unknown var %s\n", unit->get_var ().c_str ());
                        abort ();
                    }
                    else
                        intermediate_st.push (find_res->second);
                } break;

                case type::OP: {
                    auto cur_op = unit->get_op ();
                    if (cur_op == op::CALL || cur_op == op::TERN)
                        return false;
                    if (cur_op == op::ENDTRUE || cur_op == op::ENDFALSE)
                        return true;
                    if (cur_op == op::ENDCOND) {       // tern
                        if (!intermediate_st.top ())            // if false
                            for (;unit->get_type () != type::OP || unit->get_op () != op::ENDTRUE; unit = expr.top ())
                                expr.pop ();
                        break;                            
                    }
                    auto right = intermediate_st.top ();
                    intermediate_st.pop ();
                    auto left  = intermediate_st.top ();
                    intermediate_st.pop ();
                    switch (unit->get_op ()) {
                        default: return false; break;
                        case op::ADD:       intermediate_st.push (left + right);  break;
                        case op::SUB:       intermediate_st.push (left - right);  break;
                        case op::MUL:       intermediate_st.push (left * right);  break;
                        case op::DIV:       intermediate_st.push (left / right);  break;
                        case op::MORE:      intermediate_st.push (left > right);  break;
                        case op::LESS:      intermediate_st.push (left < right);  break;
                        case op::MOREOREQ:  intermediate_st.push (left >= right); break;
                        case op::LESSOREQ:  intermediate_st.push (left <= right); break;
                        case op::EQUAL:     intermediate_st.push (left == right); break; 
                        case op::NOTEQUAL:  intermediate_st.push (left != right); break;  
                    }
                } break;
            }
            expr.pop ();
        }
        return true;
    }
    

    void Interpreter::update_htable (const ast::IAST* var_list, decltype (var_value_) old_htable) {
        assert (var_list);
        assert (var_list->get_type () == VAR);

        auto var_str = var_list->get_var ();

        if (var_str[0] == '*') {
            for (const auto& elem : var_value_) {
                auto find_res = old_htable.find (elem.first);
                if (find_res != old_htable.end ())
                    find_res->second = elem.second;
            }
        }
        else {
            unsigned begin = 0;
            unsigned end   = 0;
            while (var_str.length () > end) {
                while (var_str[end] != ',' && var_str[end] != '\0')
                    end++;

                std::string var (var_str, begin, end - begin);
                end++;
                begin = end;

                auto find_res = var_value_.find (var);
                assert (find_res != var_value_.end ());
                auto find_res_old = old_htable.find(var);
                if (find_res_old != old_htable.end ())
                    find_res_old->second = find_res->second;
            }
        }

        var_value_ = old_htable;
    }


    void Interpreter::create_htable (const ast::IAST* var_list) {
        assert (var_list);
        assert (var_list->get_type () == VAR);

        auto var_str = var_list->get_var ();
        auto old_htable = var_value_;
        var_value_.clear ();

        unsigned begin = 0;
        unsigned end   = 0;
        while (var_str.length () > end) {
            if (var_str[begin] == '*') {
                var_value_ = old_htable;
                break;
            }
            while (end < var_str.length () && var_str[end] != ',')
                end++;

            std::string var (var_str, begin, end - begin);
            end++;
            begin = end;

            auto find_res = old_htable.find (var);
            if (find_res == old_htable.end ()) {
                printf ("\nunknown var '%s' in capture block\n", var.c_str ());
                abort ();
            }
            else
                var_value_.insert (std::make_pair (var, find_res->second));
        }

    }
}
