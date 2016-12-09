#include "parser.h"
#include <cassert>

namespace parser {

    IAST* Parser::build () {
        root_ = muldiv_parse ();

        return root_->clone ();
    } 


    IAST* Parser::muldiv_parse () {
        using namespace op;
        IAST* ast = vlvr_parse ();

        ILexem* lex = lxr_.cur_lexem ();
        while (lex->get_type () == OP && (lex->get_op () == MUL || lex->get_op () == DIV)) {
            Operator oper = lex->get_op ();
            lxr_.next_lexem ();

            IAST* right = vlvr_parse ();
            ast = new Op_AST (oper, ast, right);

            lex = lxr_.cur_lexem ();
        }

        return ast;
    }


    IAST* Parser::vlvr_parse () {
        ILexem* l = lxr_.cur_lexem ();  

        if (l->get_type () == VAL)
            return val_parse ();
        else if (l->get_type () == VAR)
            return var_parse ();
        else
            assert (0);
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

        IAST* ast = new Var_AST (l->get_var ());
        delete l;
        return ast;
    }



}