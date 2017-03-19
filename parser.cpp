#include "parser.h"
#include <cassert>
#include <list>

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
        work_on_cond_op (repetitive_.top (), op::WHILE);
        repetitive_.pop ();
    }

    void Parser::skip ()
    {
        if (parts_.empty ())
            return;
        auto tmp = parts_.top ();
        parts_.pop ();
        while (1)
        {
            if (parts_.empty ())
                return;
            if (tmp->get_type () == VAL || tmp->get_type () == VAR ||
                tmp->get_op () != op::ENDWHILE  || tmp->get_op () != op::ENDIF)
            {
                tmp = parts_.top ();
                parts_.pop ();
                continue;
            }
            else
                return;
        }
    }

    void Parser::work_on_cond_op (const ast::IAST *node, op::Operator op_type)
    {
        assert(op_type == IF || op_type == WHILE);
        if (op_type == WHILE)
        {
            assert (node);
            repetitive_.push (node);
            parts_.push (new Op_AST (op::ENDWHILE, nullptr, nullptr));
        }
        else
            parts_.push (new Op_AST (op::ENDIF, nullptr, nullptr));
        
        auto tmp = node->get_right ()->get_right ();
        auto type = tmp->get_op ();
        bool broken = false;
        while (!broken)
        {
            switch (type)
            {
                case CODE:  parts_.push (tmp->get_right ());
                            tmp = tmp->get_left ();
                            type = tmp->get_op ();
                            break;

                default:    parts_.push (tmp);
                            broken = true;
                            break;
            }
        } 
        parts_.push (node->get_right ()->get_left ());
        parts_.push (node->get_left ()); 
    }

    IAST const* Parser::get_next ()
    {
        if (parts_.empty ())
        {
            build();
            if (last_part_ == nullptr)
                return nullptr;
            auto cur_type = last_part_->get_op ();
            switch (cur_type)
            {
                case ASSIGN:    parts_.push (last_part_->get_right ());
                                parts_.push (last_part_->get_left ());
                                return last_part_;

                case IF:        work_on_cond_op (last_part_, op::IF); 
                                return last_part_;

                case WHILE:     work_on_cond_op (last_part_, op::WHILE);
                                return last_part_;
            
                default:        assert(!"expect IF, WHILE or ASSIGN here");
            }
        }
        else
        {
            auto tmp = parts_.top ();
            parts_.pop ();
            auto general_type = tmp->get_type ();
            if (general_type == Type::VAR || general_type == Type::VAL)
                return tmp;
            auto cur_type = tmp->get_op ();
            switch (cur_type)
            {
                case ASSIGN:    parts_.push (tmp->get_right ());
                                parts_.push (tmp->get_left ());
                                return tmp;

                case IF:        work_on_cond_op (tmp, op::IF);
                                return tmp;

                case WHILE:     work_on_cond_op (tmp, op::WHILE);
                                return tmp;
                
                case ENDWHILE...ENDIF:  
                                cur_deep_diff_ = 1;
                                deep_decreased_ = true;
                                if (status_ == INTERPRETER)
                                    return tmp;
                                delete tmp;
                                while (!parts_.empty ())
                                {
                                    tmp = parts_.top ();
                                    cur_type = tmp->get_op ();
                                    if (cur_type == ENDIF || cur_type == ENDWHILE /*|| cur_type == ENDFUNC*/)
                                    {
                                        delete tmp;
                                        ++cur_deep_diff_;
                                        parts_.pop ();
                                    }
                                    else
                                        break;//return tmp;
                                }
                                return get_next ();

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

        return;
    } 

    IAST* Parser::code_parse () {
        static int controller = 0;
        ++controller;
    
        if (controller > 1)
        {
            auto tree = function_parse ();//
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
        if (!cur_lexem.is_assign ()) {
            printf ("\nexpected '=' at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        auto right = tern_parse ();
        if (!right) {
            printf ("\nsyntax error at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }

        cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_semicolon ()) {
            printf ("\nexpected ';' before line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        return new Op_AST (ASSIGN, left, right);
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
            res = var_parse ();
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


    IAST* Parser::var_parse () {
        auto cur_lexem = lxr_.get_cur_lexem ();
        lxr_.next_lexem ();

        if (cur_lexem.get_type () != VAR) {
            printf ("\nexpected primary-expression before ");
            if (cur_lexem.get_type () == VAL)
                printf ("'%lg'", cur_lexem.get_val ());
            else
                printf ("'%s'", string_eq (cur_lexem.get_op ()));
            printf (" at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }

        return new Var_AST (cur_lexem.get_var ());
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
}
