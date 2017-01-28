#include "parser.h"
#include <cassert>

namespace parser {

    IAST* Parser::build () {
        if (root_)
            return root_->clone ();

        root_ = assign_parse ();

        Lexem l = lxr_.cur_lexem ();
        while (l.get_type () == OP && l.get_op () == op::SMCN) {
            lxr_.next_lexem ();
            Lexem new_l = lxr_.cur_lexem ();
            if (new_l.get_type () == OP && new_l.get_op () == op::END)
                break;
            l = new_l;
            if (l.get_type () == OP && l.get_op () == op::SMCN)
                continue;

            IAST* right = assign_parse ();

            root_ = new Op_AST (op::CODE, root_, right);
            l = lxr_.cur_lexem ();
        }

        if (l.get_type () != OP || l.get_op () != op::SMCN) {
            printf ("\nexpected ';' before line %u, pos %u\n\n", l.get_line (), l.get_pos ());
            abort ();
        }

        if (!root_) {
            printf ("\nsyntax error at line %u, pos %u\n\n", l.get_line (), l.get_pos ());
            abort ();
        }   

        printf ("\tParser: the end of the program was reached at line %u, pos %u\n", l.get_line (), l.get_pos ());
        return root_->clone ();
    } 


    IAST* Parser::assign_parse () {
        using namespace op;
        IAST* left = var_parse ();

        Lexem lex = lxr_.cur_lexem ();
        if (lex.get_type () != OP || lex.get_op () != ASSIGN) {
            printf ("\nexpected '=' at line %u, pos %u\n\n", lex.get_line (), lex.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        IAST* right = tern_parse ();
        if (!right) {
            printf ("\nsyntax error at line %u, pos %u\n\n", lex.get_line (), lex.get_pos ());
            abort ();
        }

        return new Op_AST (ASSIGN, left, right);
    }


    IAST* Parser::tern_parse () {
        using namespace op;
        IAST* cond = cond_parse ();

        Lexem lex = lxr_.cur_lexem ();
        if (lex.get_type () != OP || lex.get_op () != QUESTION) {
            IAST* residual_part = addsub_parse (cond);
            return residual_part;
        }
        lxr_.next_lexem ();
        IAST* iftrue = addsub_parse ();

        lex = lxr_.cur_lexem ();
        if (lex.get_type () != OP || lex.get_op () != COLON) {
            printf ("\nsyntax error at line %u, pos %u\n\n", lex.get_line (), lex.get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        IAST* iffalse = addsub_parse ();

        IAST* code = new Op_AST (CODE, iftrue, iffalse);
        return new Op_AST (TERN, cond, code);
    }


    IAST* Parser::cond_parse () {
        using namespace op;
        Lexem lex = lxr_.cur_lexem ();
        bool brackets = false;
        if (lex.get_type () == OP && lex.get_op () == OBRT) {
            lxr_.next_lexem ();
            brackets = true;
        }

        IAST* left = addsub_parse ();
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
        IAST* right = addsub_parse ();

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

        Lexem lex = lxr_.cur_lexem ();
        while (lex.get_type () == OP && (lex.get_op () == ADD || lex.get_op () == SUB)) {
            Operator oper = lex.get_op ();
            lxr_.next_lexem ();

            IAST* right = muldiv_parse ();
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

        Lexem lex = lxr_.cur_lexem ();
        while (lex.get_type () == OP && (lex.get_op () == MUL || lex.get_op () == DIV)) {
            Operator oper = lex.get_op ();
            lxr_.next_lexem ();

            IAST* right = bracket_parse ();
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
        Lexem lex = lxr_.cur_lexem ();
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


    IAST* Parser::val_parse () {
        Lexem l = lxr_.cur_lexem ();
        lxr_.next_lexem ();

        IAST* ast = new Val_AST (l.get_val ());
        return ast;

    }


    IAST* Parser::var_parse () {
        Lexem l = lxr_.cur_lexem ();
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

        IAST* ast = new Var_AST (l.get_var ());

        return ast;
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