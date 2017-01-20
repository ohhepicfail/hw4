#include "parser.h"
#include <cassert>

namespace parser {

    IAST* Parser::build () {
        if (root_)
            return root_->clone ();

        root_ = assign_parse ();

        ILexem* l = lxr_.cur_lexem ();
        while (l->get_type () == OP && l->get_op () == op::SMCN) {
            lxr_.next_lexem ();
            ILexem* new_l = lxr_.cur_lexem ();
            if (new_l->get_type () == OP && new_l->get_op () == op::END) {
                delete new_l;
                break;
            }
            delete l;
            l = std::move (new_l);
            if (l->get_type () == OP && l->get_op () == op::SMCN)
                continue;

            IAST* right = assign_parse ();

            root_ = new Op_AST (op::CODE, root_, right);
            delete l;
            l = lxr_.cur_lexem ();
        }

        if (l->get_type () != OP || l->get_op () != op::SMCN) {
            printf ("\nexpected ';' before line %u, pos %u\n\n", l->get_line (), l->get_pos ());
            abort ();
        }

        if (!root_) {
            printf ("\nsyntax error at line %u, pos %u\n\n", l->get_line (), l->get_pos ());
            abort ();
        }   

        printf ("\nParser:\nthe end of the program was reached at line %u, pos %u\n\n", l->get_line (), l->get_pos ());
        delete l;

        return root_->clone ();
    } 


    IAST* Parser::assign_parse () {
        using namespace op;
        IAST* left = var_parse ();

        ILexem* lex = lxr_.cur_lexem ();
        if (lex->get_type () != OP || lex->get_op () != ASSIGN) {
            printf ("\nexpected '=' at line %u, pos %u\n\n", lex->get_line (), lex->get_pos ());
            abort ();
        }
        lxr_.next_lexem ();

        IAST* right = addsub_parse ();
        if (!right) {
            printf ("\nsyntax error at line %u, pos %u\n\n", lex->get_line (), lex->get_pos ());
            abort ();
        }

        delete lex;
        return new Op_AST (ASSIGN, left, right);
    }


    IAST* Parser::addsub_parse () {
        using namespace op;
        IAST* ast = muldiv_parse ();

        ILexem* lex = lxr_.cur_lexem ();
        while (lex->get_type () == OP && (lex->get_op () == ADD || lex->get_op () == SUB)) {
            Operator oper = lex->get_op ();
            lxr_.next_lexem ();

            IAST* right = muldiv_parse ();
            if ((ast == nullptr && oper != op::SUB) || right == nullptr) {
                printf ("\nsyntax error at line %u, pos %u\n\n", lex->get_line (), lex->get_pos ());
                abort ();
            }
            ast = new Op_AST (oper, ast, right);

            delete lex;
            lex = lxr_.cur_lexem ();
        }

        delete lex;
        return ast;
    }


    IAST* Parser::muldiv_parse () {
        using namespace op;
        IAST* ast = bracket_parse ();

        ILexem* lex = lxr_.cur_lexem ();
        while (lex->get_type () == OP && (lex->get_op () == MUL || lex->get_op () == DIV)) {
            Operator oper = lex->get_op ();
            lxr_.next_lexem ();

            IAST* right = bracket_parse ();
            if (ast == nullptr || right == nullptr) {
                printf ("\nsyntax error at line %u, pos %u\n\n", lex->get_line (), lex->get_pos ());
                abort ();
            }
            ast = new Op_AST (oper, ast, right);

            delete lex;
            lex = lxr_.cur_lexem ();
        }

        delete lex;
        return ast;
    }


    IAST* Parser::bracket_parse () {
        ILexem* lex = lxr_.cur_lexem ();
        IAST* ast = nullptr;
        if (lex->get_type () == OP && lex->get_op () == op::OBRT) {
            delete lex;
            lxr_.next_lexem ();
            ast = addsub_parse ();

            lex = lxr_.cur_lexem ();
            if (lex->get_type () != OP || lex->get_op () != op::CBRT) {
                printf ("\nexpected ')' before line %u, pos %u\n\n", lex->get_line (), lex->get_pos ());
                abort ();
            } 
            lxr_.next_lexem ();
        }
        else
            ast = vlvr_parse ();

        delete lex;
        return ast;
    }


    IAST* Parser::vlvr_parse () {
        ILexem* l = lxr_.cur_lexem ();  

        IAST* res = nullptr;
        if (l->get_type () == VAL)
            res = val_parse ();
        else if (l->get_type () == VAR)
            res = var_parse ();

        delete l;
        return res;
    }


    IAST* Parser::val_parse () {
        ILexem* l = lxr_.cur_lexem ();
        lxr_.next_lexem ();

        IAST* ast = new Val_AST (l->get_val ());
        delete l;
        return ast;

    }


    IAST* Parser::var_parse () {
        ILexem* l = lxr_.cur_lexem ();
        lxr_.next_lexem ();

        if (l->get_type () != VAR) {
            printf ("\nexpected primary-expression before ");
            if (l->get_type () == VAL)
                printf ("'%lg'", l->get_val ());
            else
                printf ("'%s'", string_eq (l->get_op ()));
            printf (" at line %u, pos %u\n\n", l->get_line (), l->get_pos ());
            abort ();
        }

        IAST* ast = new Var_AST (l->get_var ());

        delete l;
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