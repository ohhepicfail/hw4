#include "interpreter.h"
#include <list>
#include <string>

namespace ipr {
    // struct Node_info {
    //     const ast::IAST*  node_;
    //     bool need_to_visit_;
    // };
    
    // std::stack<Node_info> build_expr_stack (const ast::IAST*  node);


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
        std::stack <scope_data> scope;
        auto cur_node = parser_.get_next ();
        for (;;) {
            if (cur_node->get_type () != type::OP) {
                printf ("Statement has no effect\n");
                abort ();
            }
            switch (cur_node->get_op ()) {
                default:
                    assert (0);
                break;

                case op::WHILE:
                case op::IF: {
                    auto expr = parser_.get_next_expr ();
                    std::stack<double> intermediate_st;
                    auto completed = calculate_expr (expr, intermediate_st);
                    assert (completed);
                
                    if (intermediate_st.top ()) {
                        cur_node = parser_.get_next ();
                        scope.push (scope_data (var_value_, cur_node));
                        create_htable (cur_node);                
                    }
                    else
                        parser_.skip ();
                } break;

                case op::ENDIF: {
                    if (scope.empty ()) {
                        printf ("without previous if\n");
                        abort ();
                    }
                    update_htable (scope.top ().var_list_, scope.top ().htable_);
                    scope.pop ();
                } break;

                case op::ENDWHILE: {
                    if (scope.empty ()) {
                        printf ("without previous while\n");
                        abort ();
                    }

                    parser_.repeat ();
                    auto expr = parser_.get_next_expr ();
                    std::stack<double> intermediate_st;
                    auto completed = calculate_expr (expr, intermediate_st);
                    assert (completed);

                    if (!intermediate_st.top ()) {
                        parser_.skip ();
                        update_htable (scope.top ().var_list_, scope.top ().htable_);
                        scope.pop ();
                    }
                    else
                        parser_.get_next ();    // skip capture
                    
                } break;

                case op::ASSIGN: {
                    auto var = parser_.get_next ();
                    assert (var);
                    assert (var->get_type () == type::VAR); 

                    auto expr = parser_.get_next_expr ();
                    std::stack<double> intermediate_st;
                    auto completed = calculate_expr (expr, intermediate_st);
                    
                    // tern or func (because there is something different from +, -, *, /
                    if (!completed) {       
                        auto cond_result = intermediate_st.top ();
                        intermediate_st.pop ();

                        if (cond_result) {
                            expr.pop ();         // ENDCOND
                            calculate_expr (expr, intermediate_st);
                            if (expr.top ()->get_op () == op::FUNCTION)
                                assert (0);
                            else if (expr.top ()->get_op () != op::ENDTRUE)
                                assert (0);
                                                   
                        }
                        else {
                            while (expr.top ()->get_type () != type::OP
                                || expr.top ()->get_op ()   != op::ENDTRUE)
                                    expr.pop (); 
                            expr.pop ();     // ENDTRUE;
    
                            calculate_expr (expr, intermediate_st);
                            if (expr.top ()->get_op () == op::FUNCTION)
                                assert (0);
                            else if (expr.top ()->get_op () != op::ENDFALSE)
                                assert (0);
                        }
                    }
                    auto res = intermediate_st.top ();
                    auto find_res = var_value_.find (var->get_var ());
                    if (find_res == var_value_.end ())
                        var_value_.insert (std::make_pair (var->get_var (), res));
                    else
                        find_res->second = res;
                    
                } break;
            }
            
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
                    if (intermediate_st.size () <= 1)
                        return false;               // bad op
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
