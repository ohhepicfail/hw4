#ifndef PARSER_H
#define PARSER_H

#include <stack>
#include <string>
#include "lexer.h"
#include "ast.h"
#include <unordered_map>

using namespace lexem;
using namespace ast;

namespace parser {
    
    enum Status {
        INTERPRETER,
        TRANSLATOR
    };


    class Parser {
    private:
        Lexer   lxr_;
        IAST    *root_ = nullptr;
        IAST    const *last_part_ = nullptr;
    
        unsigned    cur_deep_diff_ = 0;
        bool        deep_decreased_ = false, 
                    endwhile_poped = false; 
        Status      status_;
        std::stack<const ast::IAST*> expr_;
        std::stack<const ast::IAST*> repetitive_;
        std::stack<const ast::IAST*> parts_;
        std::unordered_map<std::string, const ast::IAST*> funcs_;
        std::unordered_map<std::string, const ast::IAST*> arrays_;
        std::stack<decltype(arrays_)> arrays_scope_;
        void fill_expr (const ast::IAST* node);
        void extract_body (const ast::IAST*, op::Operator op_type);
        void add_func (const ast::IAST* func);

        IAST* code_parse ();
        IAST* function_parse ();
        IAST* while_parse ();
        IAST* if_parse ();
        IAST* assign_parse ();
        IAST* tern_parse ();
        IAST* cond_parse ();
        IAST* addsub_parse (IAST* left = nullptr);
        IAST* muldiv_parse (IAST* left = nullptr);
        IAST* bracket_parse ();
        IAST* capture_parse (const IAST* cond_vars = nullptr);
        IAST* func_param_parse ();
        IAST* vlvr_parse ();
        IAST* func_call_parse ();
        IAST* val_parse ();
        IAST* var_parse ();
        IAST* array_access_parse(IAST* left);
        IAST* array_def_parse(IAST* left);
        IAST* var_list_parse (IAST* args = nullptr);

        IAST* get_all_subtree_var (const IAST* subtree);

        void get_func_params (std::string& func_params);

        void fix_arrays_scope(const IAST* subtree);

    public:
        explicit Parser (const char* filename, Status status) : lxr_ (filename), status_(status) {}
        ~Parser () { delete root_; }
        Parser (const Parser& that) : lxr_ (that.lxr_), root_ (that.root_ ? that.root_->clone () : nullptr) {}
        Parser (Parser&& that) : lxr_ (std::move (that.lxr_)), root_ (that.root_) { that.root_ = nullptr;}
        Parser& operator= (const Parser& that);
        Parser& operator= (Parser&& that);
        const IAST* get_next_stmt(); 
        IAST const* get_next (); 
        std::stack<const ast::IAST*>&& get_next_expr ();
        void load_func (const ast::IAST* func);
        void repeat ();
        void skip ();
        bool deep_decreased ();
        unsigned get_deep_change ();
        void build ();
    };

}


#endif 
