#include "parser.h"
#include <cassert>
#include <list>

namespace parser {

    IAST* Parser::build () {
        if (root_)
            return root_->clone ();

        root_ = code_parse ();

        auto cur_lexem = lxr_.get_cur_lexem ();
        if (!root_) {
            printf ("\nsyntax error at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }   

        printf ("\tParser: the end of the program was reached at line %u, pos %u\n", cur_lexem.get_line (), cur_lexem.get_pos ());
        return root_->clone ();
    } 


    IAST* Parser::code_parse () {
        auto tree = while_parse ();

        auto cur_lexem = lxr_.get_cur_lexem ();
        while (!cur_lexem.is_closing_operator ()) {
            if (cur_lexem.is_semicolon ()) {
                lxr_.next_lexem ();
                cur_lexem = lxr_.get_cur_lexem ();
                continue;
            }

            auto right = while_parse ();

            tree = new Op_AST (op::CODE, tree, right);
            cur_lexem = lxr_.get_cur_lexem ();
        }

        return tree;
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

        decltype (cur_lexem.get_var ()) var1 = nullptr;
        decltype (cur_lexem.get_var ()) var2 = nullptr;
        auto need_to_clean = false;
        if (cond_vars) {
            var1 = get_all_subtree_var (cond_vars->get_left ());
            var2 = get_all_subtree_var (cond_vars->get_right ());
            need_to_clean = true;
        }

        lxr_.next_lexem ();
        cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_open_bracket ()) {
            printf ("\nexpected '(' after 'capture' at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }

        lxr_.next_lexem ();
        cur_lexem = lxr_.get_cur_lexem ();
        unsigned n = 256;
        auto var_list = new char[n] ();
        unsigned cur_n = 0;
        while (cur_lexem.get_type () == VAR) {
            if (!var1 && var2) {
                delete[] var1;
                var1 = var2;
                var2 = nullptr;
            }
            else if (!var2) {
                if (need_to_clean)
                    delete[] var1;
                need_to_clean = false;
                var1 = cur_lexem.get_var ();
            }
            auto var_len = strlen (var1);
            if (n <= cur_n + var_len + 1) {
                n *= 2;
                auto new_list = new char[n];
                std::copy (var_list, var_list + cur_n, new_list);
                delete[] var_list;
                var_list = new_list;
            }

            std::copy (var1, var1 + var_len, var_list + cur_n);
            cur_n += var_len;

            if (!need_to_clean) {
                lxr_.next_lexem ();
                cur_lexem = lxr_.get_cur_lexem ();
                if (cur_lexem.get_type () != OP || cur_lexem.get_op () != COMMA)
                    break;
                else
                    var_list[cur_n++] = ',';

                lxr_.next_lexem ();
                cur_lexem = lxr_.get_cur_lexem ();
            }
            var1 = nullptr;
        }

        if (strlen (var_list) == 0) {
            if (cur_lexem.is_mul ()) {
                lxr_.next_lexem ();
                var_list[0] = '*';
            }
        }

        cur_lexem = lxr_.get_cur_lexem ();
        if (!cur_lexem.is_close_bracket ()) {
            printf ("\nexpected ')' in capture block at line %u, pos %u\n\n", cur_lexem.get_line (), cur_lexem.get_pos ());
            abort ();
        }

        lxr_.next_lexem ();
        auto ast = new Var_AST (var_list);
        delete[] var_list;
        return ast;
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


    char* Parser::get_all_subtree_var (const IAST* subtree) {
        assert (subtree);

        std::list<decltype (subtree)> nodes;

        auto cur = subtree;
        while (cur->get_type () == type::OP) {
            nodes.push_back (cur);
            cur = cur->get_left ();
        }

        decltype (subtree->get_var ()) var = nullptr;
        if (!nodes.empty () && cur->get_type () == VAR)
            var = cur->get_var ();
        else if (nodes.empty () && subtree->get_type () == VAR)
            var = subtree->get_var ();

        unsigned n = 256;
        auto var_list = new char[n] ();
        unsigned cur_n = 0;
        while (!nodes.empty ()) {
            if (!var) {
                cur = nodes.back ();
                cur = cur->get_right ();
                nodes.pop_back ();
                while (cur->get_type () == type::OP) {
                    nodes.push_back (cur);
                    cur = cur->get_left ();
                }

                if (cur->get_type () == VAR)
                    var = cur->get_var ();
                else
                    continue;
            }

            auto var_len = strlen (var);
            if (n <= cur_n + var_len + 1) {
                n *= 2;
                auto new_list = new char[n];
                std::copy (var_list, var_list + cur_n, new_list);
                delete[] var_list;
                var_list = new_list;
            }

            std::copy (var, var + var_len, var_list + cur_n);
            cur_n += var_len;
            var_list[cur_n++] = ',';

            var = nullptr;
        }

        return var_list;
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