#ifndef PARSER_H
#define PARSER_H

#include <string>
#include "lexer.h"
#include "ast.h"


using namespace lexem;
using namespace ast;

namespace parser {


    class Parser {
    private:
        Lexer lxr_;
        IAST* root_ = nullptr;

        IAST* code_parse ();
        IAST* while_parse ();
        IAST* if_parse ();
        IAST* assign_parse ();
        IAST* tern_parse ();
        IAST* cond_parse ();
        IAST* addsub_parse (IAST* left = nullptr);
        IAST* muldiv_parse (IAST* left = nullptr);
        IAST* bracket_parse ();
        IAST* capture_parse (const IAST* cond_vars = nullptr);
        IAST* vlvr_parse ();
        IAST* val_parse ();
        IAST* var_parse ();

        void get_all_subtree_var (const IAST* subtree, std::string& res_var);
        void get_var_list (std::string& res_var_list);

    public:
        explicit Parser (const char* filename) : lxr_ (filename) {}
        ~Parser () { delete root_; }
        Parser (const Parser& that) : lxr_ (that.lxr_), root_ (that.root_ ? that.root_->clone () : nullptr) {}
        Parser (Parser&& that) : lxr_ (std::move (that.lxr_)), root_ (that.root_) { that.root_ = nullptr;}
        Parser& operator= (const Parser& that);
        Parser& operator= (Parser&& that);


        IAST* build ();
    };

}


#endif 