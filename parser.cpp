#include "parser.h"
#include <cassert>
#include <sstream>
#include <list>
#include <vector>

namespace parser {

    unsigned Parser::get_deep_change () {
        auto tmp = cur_deep_diff_;
        cur_deep_diff_ = 0;
        return tmp;
    }

    bool Parser::deep_decreased () {
        auto tmp = deep_decreased_;
        deep_decreased_ = false;
        return tmp;
    }
    
    void Parser::repeat ()
    {
        assert (!repetitive_.empty ());
        extract_body (repetitive_.top (), op::WHILE);
        repetitive_.pop ();
    }

    void Parser::skip ()
    {
        if (parts_.empty ())
            return;
        if (endwhile_poped)
        {
            endwhile_poped = false;
            repetitive_.pop ();
        }
        auto tmp = parts_.top ();
        parts_.pop ();
        while (1)
        {
            if (parts_.empty ())
                return;
            if  (tmp->get_type () == VAL || tmp->get_type () == VAR ||
                (tmp->get_op () != op::ENDWHILE  && tmp->get_op () != op::ENDIF && tmp->get_op () != op::ENDFUNC))
            {
                tmp = parts_.top ();
                parts_.pop ();
                continue;
            }
            else
                return;
        }
    }

    std::stack<const ast::IAST*>&& Parser::get_next_expr () {
        return std::move(expr_);
    }

    void Parser::load_func (const ast::IAST* func)
    {
        assert (func);
        assert (func->get_type () == OP && func->get_op () == op::CALL);
        auto iter = funcs_.find (func->get_left ()->get_var ());
        if (iter == funcs_.end ())
        {
            printf ("Function '%s' was not defined yet\n", func->get_left ()->get_var ().data ());
            abort ();
        }
        extract_body (iter->second, op::FUNCTION);
        parts_.pop ();
    }

    void Parser::add_func (const ast::IAST* func)
    {
        assert (func);
        assert (std::get<1> (funcs_.insert (std::pair<const std::string, const ast::IAST*> (func->get_left ()->get_var (), func))));
    }

    void Parser::fill_expr (const ast::IAST* node)
    {
        assert (node);
        std::stack<const ast::IAST*>().swap (expr_);
        std::stack<const ast::IAST*> carry;
        while (1)
        {
            if (node->get_type () == Type::OP)
            {
                switch (node->get_op ())
                {
                    case TERN:  carry.push (node->get_left ());
                                carry.push (new Op_AST (op::ENDCOND, nullptr, nullptr));
                                carry.push (node->get_right ()->get_left ());
                                carry.push (new Op_AST (op::ENDTRUE, nullptr, nullptr));
                                carry.push (node->get_right ()->get_right ());
                                expr_.push (new Op_AST (op::ENDFALSE, nullptr, nullptr));
                                node = carry.top ();
                                carry.pop ();
                                break;
                    case ENDTRUE:
                    case ENDFALSE:
                    case ENDCOND:   expr_.push (node);
                                    node = carry.top ();
                                    carry.pop ();
                                    break;
                    case CALL:  expr_.push (node->get_right ());
                                expr_.push (node);
                                if (carry.empty ())
                                    return;
                                node = carry.top ();
                                carry.pop ();
                                break;                
                    default:    expr_.push (node);
                                carry.push (node->get_left ());
                                node = node->get_right ();
                                break;
                }
            }
            else
            {   
                expr_.push (node);
                if (carry.empty ())
                    return;
                node = carry.top ();
                carry.pop ();
            }
        }
    }

    void Parser::extract_body (const ast::IAST* node, op::Operator op_type)
    {
        assert (node);
        assert (op_type == IF || op_type == WHILE || op_type == FUNCTION);
        switch (op_type)
        {
            case WHILE:     assert (node);
                            repetitive_.push (node);
                            parts_.push (new Op_AST (op::ENDWHILE, nullptr, nullptr));
                            break;
            case IF:        parts_.push (new Op_AST (op::ENDIF, nullptr, nullptr));
                            break;
            case FUNCTION:  parts_.push (new Op_AST (op::ENDFUNC, nullptr, nullptr));
                            break;
            default:        assert (!"UNKNOWN");
        }
        auto tmp = node->get_right ()->get_right ();
        auto type = tmp->get_op ();
        bool from_default = false;
        while (!from_default)
        {
            switch (type)
            {
                case CODE:  parts_.push (tmp->get_right ());
                            tmp = tmp->get_left ();
                            type = tmp->get_op ();
                            break;

                default:    parts_.push (tmp);
                            from_default = true;
                            break;
            }
        }
        parts_.push (node->get_right ()->get_left ());
        if (op_type == FUNCTION)
            parts_.push (node->get_left ());
        else
            fill_expr (node->get_left ()); 
    }

    IAST const* Parser::get_next ()
    {
        if (endwhile_poped)
            endwhile_poped = false;
        if (parts_.empty ())
        {
            build();
            if (last_part_ == nullptr)
                return nullptr;
            auto cur_type = last_part_->get_op ();
            switch (cur_type)
            {
                case ASSIGN:    parts_.push (last_part_->get_left ());
                                fill_expr (last_part_->get_right ());
                                return last_part_;

                case DEF:       parts_.push (last_part_->get_left ());
                                return last_part_;

                case IF:        extract_body (last_part_, op::IF); 
                                return last_part_;

                case WHILE:     extract_body (last_part_, op::WHILE);
                                return last_part_;
            
                case FUNCTION:  add_func (last_part_);
                                if (status_ == TRANSLATOR)
                                {
                                    extract_body (last_part_, op::FUNCTION);
                                    return last_part_;
                                }
                                else
                                    return get_next ();

                default:        assert(!"expect IF, WHILE, FUNCTION or ASSIGN here");
            }
        }
        else
        {
            auto tmp = parts_.top ();
            parts_.pop ();
            auto general_type = tmp->get_type ();
            if (general_type == Type::VAR || general_type == Type::VAL || general_type == Type::ARR)
                return tmp;
            auto cur_type = tmp->get_op ();
            switch (cur_type)
            {
                case ASSIGN:    parts_.push (tmp->get_left ());
                                fill_expr (tmp->get_right ());
                                return tmp;

                case DEF:       parts_.push (last_part_->get_left ());
                                return last_part_;

                case IF:        extract_body (tmp, op::IF);
                                return tmp;

                case WHILE:     extract_body (tmp, op::WHILE);
                                return tmp;
                
                case ENDWHILE:
                case ENDIF:     cur_deep_diff_ = 1;
                                deep_decreased_ = true;
                                if (status_ == INTERPRETER)
                                    return tmp;
                                delete tmp;
                                while (!parts_.empty ())
                                {
                                    tmp = parts_.top ();
                                    cur_type = tmp->get_op ();
                                    if (cur_type == ENDIF || cur_type == ENDWHILE)
                                    {
                                        delete tmp;
                                        ++cur_deep_diff_;
                                        parts_.pop ();
                                    }
                                    else
                                        break;
                                }
                                return get_next ();

                case ENDFUNC:   return tmp;

                default:        return tmp;

            }
        }
    }

    void Parser::build () {
        auto new_part = code_parse ();
        if (new_part != nullptr)
            root_ = new_part;

        auto cur_lexem = lxr_.get_cur_lexem ();
        if (!root_) {
            printf ("\nsyntax error at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }   
        
        root_->print ("tree.dot");

        return;
    } 

    IAST* Parser::code_parse () {
        static int controller = 0;
        ++controller;
       
        if (controller > 1)
        {
            auto tree = function_parse ();
            auto cur_lexem = lxr_.get_cur_lexem ();
            while (!cur_lexem.is_closing_operator ()) {
                if (cur_lexem.is_semicolon ()) {
                    lxr_.next_lexem ();
                    cur_lexem = lxr_.get_cur_lexem ();
                    continue;
                }

                auto right = function_parse ();

                tree = new Op_AST (op::CODE, tree, right);
                cur_lexem = lxr_.get_cur_lexem ();
            }
            --controller;
            return tree;
        }
        else
        {   
            auto cur_lexem = lxr_.get_cur_lexem ();
            if (cur_lexem.is_closing_operator ())
            {
                last_part_ = nullptr;
                return nullptr;
            }
            while (cur_lexem.is_semicolon ()) //skip semicolons
            {
                lxr_.next_lexem ();
                cur_lexem = lxr_.get_cur_lexem ();
            }
            auto tree = function_parse ();
            --controller;
            last_part_ = tree;
            if (root_ == nullptr)
                return tree;
            return new Op_AST (op::CODE, root_, tree);
        }
    }


    IAST* Parser::function_parse () {
        auto cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_function ())
            return while_parse (); 

        lxr_.next_lexem ();
        auto func_name = var_parse ();
        auto params = func_param_parse ();
        
        cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_open_brace ()) {
            printf ("\nexpected '{' at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        auto func_code = code_parse ();

        cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_close_brace ()) {
            printf ("\nexpected '}' at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        return new Op_AST (FUNCTION, func_name, new Op_AST (CODE, params, func_code));
    }


    IAST* Parser::while_parse () {
        auto cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_while ())
            return if_parse ();

        lxr_.next_lexem ();
        auto cond = cond_parse ();
        auto capt = capture_parse (cond);

        cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_open_brace ()) {
            printf ("\nexpected '{' at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        auto whilecode = code_parse ();

        cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_close_brace ()) {
            printf ("\nexpected '}' at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        return new Op_AST (WHILE, cond, new Op_AST (CODE, capt, whilecode));
    }


    IAST* Parser::if_parse () {
        auto cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_if ())
            return assign_parse ();

        lxr_.next_lexem ();

        auto cond   = cond_parse ();
        auto capt   = capture_parse ();
        auto ifcode = code_parse ();

        cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_endif ()) {
            printf ("\nsyntax error at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        return new Op_AST (IF, cond, new Op_AST (CODE, capt, ifcode));
    }


    IAST* Parser::assign_parse () {
        using namespace op;
        auto left = var_parse ();

        auto cur_lexem = lxr_.get_cur_lexem ();
        IAST* op;
        if (cur_lexem.is_assign()) {
          lxr_.next_lexem ();
          auto right = tern_parse ();
          if (!right) {
              printf ("\nsyntax error at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
              abort ();
          }

          op = new Op_AST (ASSIGN, left, right);
        }
        else if (cur_lexem.is_open_sbracket()) {
          op = array_def_parse(left);
        }
        else {
            printf ("\nexpected '='  or array definition at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }

        cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_semicolon ()) {
            printf ("\n1expected ';' before line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        return op;
    }


    IAST* Parser::tern_parse () {
        using namespace op;
        auto cond = cond_parse ();

        auto cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_question ()) {
            auto residual_part = addsub_parse (cond);
            return residual_part;
        }
        lxr_.next_lexem ();
        auto iftrue = addsub_parse ();

        cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_colon ()) {
            printf ("\nsyntax error at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        auto iffalse = addsub_parse ();

        auto code = new Op_AST (CODE, iftrue, iffalse);
        return new Op_AST (TERN, cond, code);
    }


    IAST* Parser::cond_parse () {
        using namespace op;
        auto cur_lexem = lxr_.get_cur_lexem ();
        bool brackets = false;
        if (cur_lexem.is_open_bracket ()) {
            lxr_.next_lexem ();
            brackets = true;
        }

        auto left = addsub_parse ();
        cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_comparison_operator ()) {
            if (!brackets)
                return left;

            if (cur_lexem.is_close_bracket ()) {
                lxr_.next_lexem ();
                return left;
            }
            else {
                printf ("\nsyntax error at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
                abort ();
            }
        }

        Operator cond = cur_lexem.get_op ();
        lxr_.next_lexem ();
        auto right = addsub_parse ();

        if (brackets) {
            cur_lexem = lxr_.get_cur_lexem ();
            lxr_.next_lexem ();
            if (!cur_lexem.is_close_bracket ()) {
                printf ("\nexpected ')' before line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
                abort ();
            }
        }

        return new Op_AST (cond, left, right);
    }


    IAST* Parser::addsub_parse (IAST* left) {
        using namespace op;
        if (!left)
            left = muldiv_parse ();
        else
            left = muldiv_parse (left);

        auto cur_lexem = lxr_.get_cur_lexem ();
        while (cur_lexem.is_add () || cur_lexem.is_sub ()) {
            auto oper = cur_lexem.get_op ();
            lxr_.next_lexem ();

            auto right = muldiv_parse ();
            if ((left == nullptr && oper != op::SUB) || right == nullptr) {
                printf ("\nsyntax error at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
                abort ();
            }
            left = new Op_AST (oper, left, right);

            cur_lexem = lxr_.get_cur_lexem ();
        }

        return left;
    }


    IAST* Parser::muldiv_parse (IAST* left) {
        using namespace op;
        if (!left)
            left = bracket_parse ();

        auto cur_lexem = lxr_.get_cur_lexem ();
        while (cur_lexem.is_mul () || cur_lexem.is_div ()) {
            auto oper = cur_lexem.get_op ();
            lxr_.next_lexem ();

            auto right = bracket_parse ();
            if (left == nullptr || right == nullptr) {
                printf ("\nsyntax error at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
                abort ();
            }
            left = new Op_AST (oper, left, right);

            cur_lexem = lxr_.get_cur_lexem ();
        }

        return left;
    }


    IAST* Parser::bracket_parse () {
        auto cur_lexem = lxr_.get_cur_lexem ();
        IAST* ast = nullptr;
        if (cur_lexem.is_open_bracket ()) {
            lxr_.next_lexem ();
            ast = addsub_parse ();

            cur_lexem = lxr_.get_cur_lexem ();
            if (!cur_lexem.is_close_bracket ()) {
                printf ("\nexpected ')' before line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
                abort ();
            } 
            lxr_.next_lexem ();
        }
        else
            ast = vlvr_parse ();

        return ast;
    }


    IAST* Parser::vlvr_parse () {
        auto cur_lexem = lxr_.get_cur_lexem ();  

        IAST* res = nullptr;
        if (cur_lexem.get_type () == VAL)
            res = val_parse ();
        else if (cur_lexem.get_type () == VAR)
            res = func_call_parse ();
        else if (cur_lexem.is_sub ())     // negation
            return new Val_AST (0);
        else 
            assert (0);

        return res;
    }


    IAST* Parser::capture_parse (const IAST* cond_vars) {
        auto cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_capture ())
            return new Var_AST ("*");

        std::string vars_from_cond1;
        std::string vars_from_cond2;
        if (cond_vars) {
            get_all_subtree_var (cond_vars->get_left (),  vars_from_cond1);
            get_all_subtree_var (cond_vars->get_right (), vars_from_cond2);
        }
        lxr_.next_lexem ();
        
        std::string var_list;
        get_var_list (var_list);
        vars_from_cond1 += vars_from_cond2 + var_list;

        return new Var_AST (vars_from_cond1);
    }


    IAST* Parser::func_param_parse () {
        std::string var_list;
        get_var_list (var_list);
        return new Var_AST (var_list);
    }


    IAST* Parser::val_parse () {
        auto cur_lexem = lxr_.get_cur_lexem ();
        lxr_.next_lexem ();

        return new Val_AST (cur_lexem.get_val ());
    }


    IAST* Parser::func_call_parse () {
        auto func_name = var_parse ();
        auto cur_lexem = lxr_.get_cur_lexem ();

        if (!cur_lexem.is_open_bracket ())
            return func_name;       // variable

        std::string params;
        get_func_params (params);
        return new Op_AST (CALL, func_name, new Var_AST (params));        
    }


    IAST* Parser::var_parse () {
        auto cur_lexem = lxr_.get_cur_lexem ();
        lxr_.next_lexem ();

        if (cur_lexem.get_type () != VAR) {
            printf ("\nexpected primary-expression before ");
            if (cur_lexem.get_type () == VAL) {
                std::stringstream ss;
                ss << cur_lexem.get_val();
                printf ("'%s'", ss.str().c_str());
            }
            else
                printf ("'%s'", string_eq (cur_lexem.get_op ()));
            printf (" at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }

        IAST* var = new Var_AST (cur_lexem.get_var ());
        cur_lexem = lxr_.get_cur_lexem ();
        if (arrays_.find(var->get_var()) != arrays_.end()) {
          var = arrays_[var->get_var()]->clone();
          while (cur_lexem.is_open_sbracket()) {
            var = array_access_parse(var);
            cur_lexem = lxr_.get_cur_lexem ();
          }
        }

        return var;
    }


    IAST* Parser::array_access_parse (IAST* left) {
        auto cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_open_sbracket()) {
            printf ("\nexpected '[' at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        auto idx = vlvr_parse();

        cur_lexem = lxr_.get_cur_lexem();
        if (!cur_lexem.is_close_sbracket()) {
            printf ("\nexpected ']' at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        return new Op_AST (AACCESS, left, idx);
    }


    IAST* Parser::array_def_parse (IAST* left) {
      std::vector<unsigned> dims;
      auto cur_lexem = lxr_.get_cur_lexem ();
        while (cur_lexem.is_open_sbracket()) {
          lxr_.next_lexem ();

          cur_lexem = lxr_.get_cur_lexem ();
          if (cur_lexem.get_type () != VAL) {
            printf("\nexpected array size at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort();
          }
          dims.push_back(static_cast<unsigned>(cur_lexem.get_val()));
          lxr_.next_lexem ();

          cur_lexem = lxr_.get_cur_lexem();
          if (!cur_lexem.is_close_sbracket()) {
              printf ("\nexpected ']' at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
              abort ();
          }
          lxr_.next_lexem ();
          cur_lexem = lxr_.get_cur_lexem();
        }

        IAST* arr = new Arr_AST (left->get_var(), std::move(dims));
        if (arrays_.find(arr->get_var()) != arrays_.end()) {
          printf("\nredefinition of the %s at line %u, pos %u\n\n",
              arr->get_var().c_str(), cur_lexem.get_line (), cur_lexem.get_pos ());
          abort();
        }
        arrays_[arr->get_var()] = arr;
        auto def_op = new Op_AST (DEF, arr, /*new Val_AST(0)*/ nullptr);
        delete left;
        return def_op;
    }


    void Parser::get_all_subtree_var (const IAST* subtree, std::string& res_var) {
        assert (subtree);

        std::list<decltype (subtree)> nodes;

        auto cur = subtree;
        while (cur->get_type () == type::OP) {
            nodes.push_back (cur->get_right ());
            cur = cur->get_left ();
        }
        nodes.push_back (cur);

        while (!nodes.empty ()) {
            std::string var;

            cur = nodes.back ();
            nodes.pop_back ();

            while (cur->get_type () == type::OP) {
                nodes.push_back (cur->get_right ());
                cur = cur->get_left ();
            }

            if (cur->get_type () == VAR)
                var = cur->get_var ();
            else
                continue;


            res_var += var + ',';
        }
    }

    
    void Parser::get_func_params (std::string& func_params) {
        auto cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_open_bracket ()) {
            printf ("\nexpected '(' at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }

        lxr_.next_lexem ();
        cur_lexem = lxr_.get_cur_lexem ();
        while (cur_lexem.get_type () == VAR || cur_lexem.get_type () == VAL || cur_lexem.is_sub ()) {
            if (cur_lexem.is_sub ()) {
                func_params += '-';
                lxr_.next_lexem ();
                cur_lexem = lxr_.get_cur_lexem ();
            }

            if (cur_lexem.get_type () == VAR)
                func_params += cur_lexem.get_var ();
            else
                func_params += std::to_string (cur_lexem.get_val ());
            func_params += ',';
            lxr_.next_lexem ();
            cur_lexem = lxr_.get_cur_lexem ();
            if (!cur_lexem.is_comma ())
                break;

            lxr_.next_lexem ();
            cur_lexem = lxr_.get_cur_lexem ();
        }
        if (!func_params.empty ())
            func_params.erase (func_params.end () - 1);     // last ','

        if (!cur_lexem.is_close_bracket ()) {
            printf ("\nexpected ')' at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }

        lxr_.next_lexem ();
    }


    void Parser::get_var_list (std::string& res_var_list) {
        auto cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_open_bracket ()) {
            printf ("\nexpected '(' at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }

        lxr_.next_lexem ();
        cur_lexem = lxr_.get_cur_lexem ();

        while (cur_lexem.get_type () == VAR) {
            std::string cur_var = cur_lexem.get_var ();
            res_var_list += cur_var + ',';
            lxr_.next_lexem ();
            cur_lexem = lxr_.get_cur_lexem ();
            if (!cur_lexem.is_comma ())
                break;

            lxr_.next_lexem ();
            cur_lexem = lxr_.get_cur_lexem ();
        }

        if (res_var_list.empty ()) {
            if (cur_lexem.is_mul ()) {
                lxr_.next_lexem ();
                res_var_list = '*';
            }
        }
        else
            res_var_list.erase (res_var_list.end () - 1);        //  erase last ','

        cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_close_bracket ()) {
            printf ("\nexpected ')' at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }

        lxr_.next_lexem ();
    }


    Parser& Parser::operator= (const Parser& that) {
        if (this == &that)
            return *this;

        delete root_;
        
        *this = Parser (that);

        return *this;
    }


    Parser& Parser::operator= (Parser&& that) {
        delete root_;

        root_ = that.root_;
        that.root_ = nullptr;

        lxr_ = std::move (that.lxr_);

        return *this;
    }

    const IAST* Parser::get_next_stmt()
    {
        if (!lxr_.get_cur_lexem().is_end_file())
            build();
        return root_;
    }
}
