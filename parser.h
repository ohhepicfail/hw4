#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"


using namespace lexem;
using namespace ast;

namespace parser {


    class Parser {
    private:
        Lexer lxr_;
        IAST* root_ = nullptr;

        IAST* assign_parse ();
        IAST* addsub_parse ();
        IAST* muldiv_parse ();
        IAST* vlvr_parse ();
        IAST* val_parse ();
        IAST* var_parse ();

    public:
        Parser (const char* filename) : lxr_ (filename) {}
        ~Parser () { delete root_; }


        IAST* build ();

    };

}


#endif 