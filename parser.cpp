#include "parser.h"
#include <cassert>

namespace parser {

    IAST* Parser::build () {
        root_ = assign_parse ();

        ILexem* l = lxr_.cur_lexem ();
        while (l->get_type () == OP && l->get_op () == op::EOL) {
            lxr_.next_lexem ();
            delete l;
            l = lxr_.cur_lexem ();
            if (l->get_type () == OP && l->get_op () == op::END)
                break;
            if (l->get_type () == OP && l->get_op () == op::EOL)
                continue;

            IAST* right = assign_parse ();

            root_ = new Op_AST (op::CODE, root_, right);
            delete l;
            l = lxr_.cur_lexem ();
        }

        assert (l->get_type () == OP && l->get_op () == op::END);
        delete l;

        return root_->clone ();
    } 


    IAST* Parser::assign_parse () {
        using namespace op;
        IAST* left = var_parse ();

        ILexem* lex = lxr_.cur_lexem ();
        assert (lex->get_type () != OP || lex->get_op () == ASSIGN);
        lxr_.next_lexem ();

        IAST* right = addsub_parse ();

        delete lex;
        return new Op_AST (ASSIGN, left, right);
    }


    IAST* Parser::addsub_parse () {
        using namespace op;
        IAST* ast = brt_md_parse ();

        ILexem* lex = lxr_.cur_lexem ();
        while (lex->get_type () == OP && (lex->get_op () == ADD || lex->get_op () == SUB)) {
            Operator oper = lex->get_op ();
            lxr_.next_lexem ();

            IAST* right = brt_md_parse ();
            ast = new Op_AST (oper, ast, right);

            delete lex;
            lex = lxr_.cur_lexem ();
        }
        delete lex;

        return ast;
    }


    IAST* Parser::brt_md_parse () {
        ILexem* lex = lxr_.cur_lexem ();
        IAST* ast = nullptr;
        if (lex->get_type () == OP && lex->get_op () == op::OBRT) {
            delete lex;
            lxr_.next_lexem ();
            ast = addsub_parse ();

            lex = lxr_.cur_lexem ();
            assert (lex->get_type () == OP && lex->get_op () == op::CBRT);
            lxr_.next_lexem ();
        }
        else
            ast = muldiv_parse ();

        delete lex;
        return ast;
    }


    IAST* Parser::muldiv_parse () {
        using namespace op;
        IAST* ast = brt_vlvr_parse ();

        ILexem* lex = lxr_.cur_lexem ();
        while (lex->get_type () == OP && (lex->get_op () == MUL || lex->get_op () == DIV)) {
            Operator oper = lex->get_op ();
            lxr_.next_lexem ();

            IAST* right = brt_vlvr_parse ();
            ast = new Op_AST (oper, ast, right);

            delete lex;
            lex = lxr_.cur_lexem ();
        }
        delete lex;

        return ast;
    }


    IAST* Parser::brt_vlvr_parse () {
        ILexem* lex = lxr_.cur_lexem ();
        IAST* ast = nullptr;
        if (lex->get_type () == OP && lex->get_op () == op::OBRT) {
            delete lex;
            lxr_.next_lexem ();
            ast = addsub_parse ();

            lex = lxr_.cur_lexem ();
            assert (lex->get_type () == OP && lex->get_op () == op::CBRT); 
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
        else
            assert (0);

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

        assert (l->get_type () == VAR);

        IAST* ast = new Var_AST (l->get_var ());
        delete l;
        return ast;
    }



}