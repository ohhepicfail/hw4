#include "interpreter.h"
#include <list>
#include <string>
#include <iostream>

namespace ipr {

    lexem::val_t Interpreter::run () {
        calculate ();

        auto find_res = var_value_.find ("result");
        if (find_res == var_value_.end ()) {
            printf ("No variable 'result'\n");
            abort ();
        }
        return find_res->second;
    }


    void Interpreter::calculate () {
        struct scope_args {
            scope_args (decltype (var_value_) hash_table, decltype (parser_.get_next ()) var_list) : 
                                                        htable_ (hash_table)
                                                      , var_list_ (var_list) {} 
            decltype (var_value_) htable_;
            decltype (parser_.get_next ()) var_list_;
        };
        struct function_args {
            std::remove_reference<decltype (parser_.get_next_expr ())>::type expr_;
            std::stack<lexem::val_t> helper_;
            decltype (var_value_) htable_;
            decltype (parser_.get_next ()) dst_;
            decltype (parser_.get_next ()) var_;
            function_args (decltype (expr_) expr
                         , decltype (helper_) intermediate
                         , decltype (htable_) htable
                         , decltype (dst_) dst
                         , decltype (var_) var) :
                        expr_ (expr), 
                        helper_ (intermediate),
                        htable_ (htable),
                        dst_ (dst),
                        var_ (var) {}
        };
        
        std::stack <scope_args> scope;
        std::stack<function_args> func_data;
        bool from_func = false; 
        std::remove_reference<decltype (parser_.get_next_expr ())>::type expr;
        std::stack<lexem::val_t> intermediate_st;
        auto cur_node = parser_.get_next ();
        decltype (parser_.get_next ()) var = nullptr;
    
        auto call_func = [&func_data, &expr, &intermediate_st, &var, this] (decltype (function_args::dst_) dst) {
            auto func_id = expr.top ();
            expr.pop ();
            auto input_args = expr.top ();
            expr.pop ();

            parser_.load_func (func_id);
            func_data.push (function_args (expr, intermediate_st, var_value_, dst, var));
            auto pnames = parser_.get_next ();
            create_func_htable (input_args, pnames);
        };

        auto cleanup = [&expr, &intermediate_st] {
            decltype (expr) ().swap (expr);
            decltype (intermediate_st) ().swap (intermediate_st);
        };

        for (;;) {
            if (cur_node->get_type () != type::OP) {
                printf ("Statement has no effect\n");
                abort ();
            }
            switch (cur_node->get_op ()) {
                default:
                    assert (0);
                break;

                case WHILE:
                case IF: {
                    if (!from_func) {
                        cleanup ();
                        expr = parser_.get_next_expr ();
                    }
                    auto completed = calculate_expr (expr, intermediate_st);
                    if (!completed) {
                        auto unit = expr.top ();
                        assert (unit->get_type () == type::OP && unit->get_op () == op::CALL);
                        call_func (cur_node);
                        break;
                    }
                    if (intermediate_st.top ()) {
                        cur_node = parser_.get_next ();
                        scope.push (scope_args (var_value_, cur_node));
                        create_htable (cur_node);                
                    }
                    else
                        parser_.skip ();
                } break;

                case ENDIF: {
                    if (scope.empty ()) {
                        printf ("without previous if\n");
                        abort ();
                    }
                    update_htable (scope.top ().var_list_, scope.top ().htable_);
                    scope.pop ();
                } break;

                case ENDWHILE: {
                    if (!from_func) {
                        if (scope.empty ()) {
                            printf ("without previous while\n");
                            abort ();
                        }

                        parser_.repeat ();
                        cleanup ();
                        expr = parser_.get_next_expr ();
                    }
                    auto completed = calculate_expr (expr, intermediate_st);
                    if (!completed) {
                        auto unit = expr.top ();
                        assert (unit->get_type () == type::OP && unit->get_op () == op::CALL);
                        call_func (cur_node);
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

                case ENDFUNC: {
                    from_func = true;

                    auto find_res = var_value_.find ("result");
                    if (find_res == var_value_.end ()) {
                        printf ("No variable 'result' in function\n");
                        abort ();
                    }
                    auto res = find_res->second;

                    expr = func_data.top ().expr_;
                    intermediate_st = func_data.top ().helper_;
                    var_value_ = func_data.top ().htable_;
                    cur_node = func_data.top ().dst_;
                    var = func_data.top ().var_;
                    func_data.pop ();

                    intermediate_st.push (res);
                continue;
                } break;

                case ASSIGN: {
                    if (!from_func) {
                        var = parser_.get_next ();
                        assert (var);
                        assert (var->get_type () == type::VAR); 

                        cleanup ();
                        expr = parser_.get_next_expr ();
                    }
                    auto completed = calculate_expr (expr, intermediate_st);
                    if (!completed) {
                        auto unit = expr.top ();
                        assert (unit->get_type () == type::OP && unit->get_op () == op::CALL);
                        call_func (cur_node);
                        break;
                    }
                    auto res = intermediate_st.top ();
                    auto find_res = var_value_.find (var->get_var ());
                    if (find_res == var_value_.end ())
                        var_value_.insert (std::make_pair (var->get_var (), res));
                    else
                        find_res->second = res;
                    
                } break;
            }
            
            from_func = false;
            var = nullptr;
            cur_node = parser_.get_next ();
            if (!cur_node)
                break;
        }

    }


    bool Interpreter::calculate_expr (decltype(parser_.get_next_expr ())& expr, std::stack<lexem::val_t>& intermediate_st) {
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
        auto find_var_result = var_value_.find ("result");
        if (find_var_result != var_value_.end ()) {
            auto find_in_old = old_htable.find ("result");
            if (find_in_old == old_htable.end ())
                old_htable.insert (std::make_pair ("result", find_var_result->second));
            else
                find_in_old->second = find_var_result->second;
        }


        var_value_ = old_htable;
    }

    
    void Interpreter::create_func_htable (const ast::IAST* args, const ast::IAST* params) {
        assert (args);
        assert (params);
        assert (args->get_type () == type::VAR);
        assert (params->get_type () == type::VAR);

        auto args_str   = args->get_var ();
        auto params_str = params->get_var ();
        auto old_htable = var_value_;
        var_value_.clear ();
        
        unsigned args_beg = 0;
        unsigned args_end = 0;
        unsigned param_beg = 0;
        unsigned param_end = 0;
        for (;;) {
            while (params_str.length () > param_end && params_str[param_end] != ',')
                param_end++;
            
            auto args_type = type::NAT;
            while (args_str.length () > args_end && args_str[args_end] != ',') {
                if ((isalpha (args_str[args_end]) || args_str[args_end] == '-') && (args_type == type::NAT || args_type == type::VAR))
                    args_type = VAR;
                else if ((isdigit (args_str[args_end]) || args_str[args_end] == '.' || args_str[args_end] == '-') 
                        && (args_type == type::NAT || args_type == type::VAL))
                    args_type = VAL;  
                else {
                    printf ("bad args in function\n");
                    abort ();
                } 
                args_end++;
            }
            
            auto negation = false;
            if (args_str[args_beg] == '-') {
                negation = true;
                args_beg++;
            }
            auto arg_len   = args_end -args_beg;
            auto param_len = param_end - param_beg;
            if (!param_len && !arg_len)
                return;
            else if (param_len && !arg_len) {
                printf ("too few arguments to function\n");
                abort ();
            }
            else if (!param_len && arg_len) {
                printf ("too many arguments to function\n");
                abort ();
            }

            std::string param (params_str, param_beg, param_len);
            std::string arg (args_str, args_beg, arg_len);
            param_beg = ++param_end;
            args_beg = ++args_end;
            
            lexem::val_t number = 0;
            if (args_type == VAR) {
                auto find_res = old_htable.find (arg);
                if (find_res == old_htable.end ()) {
                    printf ("\nunknown var '%s'\n", arg.c_str ());
                    abort ();
                }
                else
                    number = find_res->second;
                number = negation ? -number : number;
            }
            else 
                number = std::stod (arg);
            var_value_.insert (std::make_pair (param, number));
        }

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
