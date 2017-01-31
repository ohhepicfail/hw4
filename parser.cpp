#include "parser.h"
#include <cassert>

namespace parser {

    IAST* Parser::build () {
        if (root_)
            return root_->clone ();

        root_ = code_parse ();

        auto l = lxr_.cur_lexem ();
        if (!root_) {
            printf ("\nsyntax error at line %u, pos %u\n\n", l.get_line (), l.get_pos ());
            abort ();
        }   

        printf ("\tParser: the end of the program was reached at line %u, pos %u\n", l.get_line (), l.get_pos ());
        return root_->clone ();
    } 


    IAST* Parser::code_parse () {
        auto tree = if_parse ();

        auto l = lxr_.cur_lexem ();
        while (l.get_type () != OP || !(l.get_op () == op::END || l.get_op () == op::ENDIF)) {
            if (l.get_type () == OP && l.get_op () == op::SMCN) {
                lxr_.next_lexem ();
                l = lxr_.cur_lexem ();
                continue;
            }

            auto right = if_parse ();

            tree = new Op_AST (op::CODE, tree, right);
            l = lxr_.cur_lexem ();
        }

        return tree;
    }


    IAST* Parser::if_parse () {
        auto lex = lxr_.cur_lexem ();
        if (lex.get_type () != OP || lex.get_op () != IF)
            return assign_parse ();

        lxr_.next_lexem ();

        auto cond   = cond_parse ();
        auto capt   = capture_parse ();
        auto ifcode = code_parse ();

        lex = lxr_.cur_lexem ();
        if (lex.get_type () != OP || lex.get_op () != ENDIF) {
            printf ("\nsyntax error at line %u, pos %u\n\n", lex.get_line (), lex.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        return new Op_AST (IF, cond, new Op_AST (CODE, capt, ifcode));
    }


    IAST* Parser::assign_parse () {
        using namespace op;
        auto left = var_parse ();

        auto lex = lxr_.cur_lexem ();
        if (lex.get_type () != OP || lex.get_op () != ASSIGN) {
            printf ("\nexpected '=' at line %u, pos %u\n\n", lex.get_line (), lex.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        auto right = tern_parse ();
        if (!right) {
            printf ("\nsyntax error at line %u, pos %u\n\n", lex.get_line (), lex.get_pos ());
            abort ();
        }

        lex = lxr_.cur_lexem ();
        if (lex.get_type () != OP || lex.get_op () != op::SMCN) {
            printf ("\nexpected ';' before line %u, pos %u\n\n", lex.get_line (), lex.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        return new Op_AST (ASSIGN, left, right);
    }


    IAST* Parser::tern_parse () {
        using namespace op;
        auto cond = cond_parse ();

        auto lex = lxr_.cur_lexem ();
        if (lex.get_type () != OP || lex.get_op () != QUESTION) {
            auto residual_part = addsub_parse (cond);
            return residual_part;
        }
        lxr_.next_lexem ();
        auto iftrue = addsub_parse ();

        lex = lxr_.cur_lexem ();
        if (lex.get_type () != OP || lex.get_op () != COLON) {
            printf ("\nsyntax error at line %u, pos %u\n\n", lex.get_line (), lex.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        auto iffalse = addsub_parse ();

        auto code = new Op_AST (CODE, iftrue, iffalse);
        return new Op_AST (TERN, cond, code);
    }


    IAST* Parser::cond_parse () {
        using namespace op;
        auto lex = lxr_.cur_lexem ();
        bool brackets = false;
        if (lex.get_type () == OP && lex.get_op () == OBRT) {
            lxr_.next_lexem ();
            brackets = true;
        }

        auto left = addsub_parse ();
        lex = lxr_.cur_lexem ();
        if (lex.get_type () != OP || ! (lex.get_op () == MORE
                                     || lex.get_op () == LESS
                                     || lex.get_op () == MOREOREQ
                                     || lex.get_op () == LESSOREQ
                                     || lex.get_op () == EQUAL
                                     || lex.get_op () == NOTEQUAL)) {
            if (!brackets)
                return left;

            if (lex.get_type () == OP && lex.get_op () == CBRT) {
                lxr_.next_lexem ();
                return left;
            }
            else {
                printf ("\nsyntax error at line %u, pos %u\n\n", lex.get_line (), lex.get_pos ());
                abort ();
            }
        }

        Operator cond = lex.get_op ();
        lxr_.next_lexem ();
        auto right = addsub_parse ();

        if (brackets) {
            lex = lxr_.cur_lexem ();
            lxr_.next_lexem ();
            if (lex.get_type () != OP || lex.get_op () != CBRT) {
                printf ("\nexpected ')' before line %u, pos %u\n\n", lex.get_line (), lex.get_pos ());
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

        auto lex = lxr_.cur_lexem ();
        while (lex.get_type () == OP && (lex.get_op () == ADD || lex.get_op () == SUB)) {
            Operator oper = lex.get_op ();
            lxr_.next_lexem ();

            auto right = muldiv_parse ();
            if ((left == nullptr && oper != op::SUB) || right == nullptr) {
                printf ("\nsyntax error at line %u, pos %u\n\n", lex.get_line (), lex.get_pos ());
                abort ();
            }
            left = new Op_AST (oper, left, right);

            lex = lxr_.cur_lexem ();
        }

        return left;
    }


    IAST* Parser::muldiv_parse (IAST* left) {
        using namespace op;
        if (!left)
            left = bracket_parse ();

        auto lex = lxr_.cur_lexem ();
        while (lex.get_type () == OP && (lex.get_op () == MUL || lex.get_op () == DIV)) {
            Operator oper = lex.get_op ();
            lxr_.next_lexem ();

            auto right = bracket_parse ();
            if (left == nullptr || right == nullptr) {
                printf ("\nsyntax error at line %u, pos %u\n\n", lex.get_line (), lex.get_pos ());
                abort ();
            }
            left = new Op_AST (oper, left, right);

            lex = lxr_.cur_lexem ();
        }

        return left;
    }


    IAST* Parser::bracket_parse () {
        auto lex = lxr_.cur_lexem ();
        IAST* ast = nullptr;
        if (lex.get_type () == OP && lex.get_op () == op::OBRT) {
            lxr_.next_lexem ();
            ast = addsub_parse ();

            lex = lxr_.cur_lexem ();
            if (lex.get_type () != OP || lex.get_op () != op::CBRT) {
                printf ("\nexpected ')' before line %u, pos %u\n\n", lex.get_line (), lex.get_pos ());
                abort ();
            } 
            lxr_.next_lexem ();
        }
        else
            ast = vlvr_parse ();

        return ast;
    }


    IAST* Parser::vlvr_parse () {
        Lexem l = lxr_.cur_lexem ();  

        IAST* res = nullptr;
        if (l.get_type () == VAL)
            res = val_parse ();
        else if (l.get_type () == VAR)
            res = var_parse ();

        return res;
    }


    IAST* Parser::capture_parse () {
        auto lex = lxr_.cur_lexem ();
        if (lex.get_type () != OP || lex.get_op () != CAPTURE)
            return new Var_AST ("*");

        lxr_.next_lexem ();
        lex = lxr_.cur_lexem ();
        if (lex.get_type () != OP || lex.get_op () != OBRT) {
            printf ("\nexpected '(' after 'capture' at line %u, pos %u\n\n", lex.get_line (), lex.get_pos ());
            abort ();
        }

        lxr_.next_lexem ();
        lex = lxr_.cur_lexem ();
        unsigned n = 256;
        auto var_list = new char[n] ();
        unsigned cur_n = 0;
        while (lex.get_type () == VAR) {
            auto var = lex.get_var ();
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

            lxr_.next_lexem ();
            lex = lxr_.cur_lexem ();
            if (lex.get_type () != OP || lex.get_op () != COMMA)
                break;
            else
                var_list[cur_n++] = ',';

            lxr_.next_lexem ();
            lex = lxr_.cur_lexem ();
        }

        if (strlen (var_list) == 0) {
            if (lex.get_type () == OP && lex.get_op () == MUL) {
                lxr_.next_lexem ();
                var_list[0] = '*';
            }
        }

        lex = lxr_.cur_lexem ();
        if (lex.get_type () != OP || lex.get_op () != CBRT) {
            printf ("\nexpected ')' in capture block at line %u, pos %u\n\n", lex.get_line (), lex.get_pos ());
            abort ();
        }

        lxr_.next_lexem ();
        auto ast = new Var_AST (var_list);
        delete[] var_list;
        return ast;
    }


    IAST* Parser::val_parse () {
        auto l = lxr_.cur_lexem ();
        lxr_.next_lexem ();

        return new Val_AST (l.get_val ());
    }


    IAST* Parser::var_parse () {
        auto l = lxr_.cur_lexem ();
        lxr_.next_lexem ();

        if (l.get_type () != VAR) {
            printf ("\nexpected primary-expression before ");
            if (l.get_type () == VAL)
                printf ("'%lg'", l.get_val ());
            else
                printf ("'%s'", string_eq (l.get_op ()));
            printf (" at line %u, pos %u\n\n", l.get_line (), l.get_pos ());
            abort ();
        }

        return new Var_AST (l.get_var ());
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