#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"
#include <stack>
#include <unordered_map>

namespace ipr {

    class Interpreter {
    private:
        parser::Parser parser_;
        std::unordered_map <std::string, lexem::val_t> var_value_;

        void   calculate ();
        bool   calculate_expr (decltype(parser_.get_next_expr ())& expr, std::stack<lexem::val_t>& intermediate_st);

        void create_func_htable (const ast::IAST* args, const ast::IAST* params); 
        void create_htable (const ast::IAST* var_list);
        void update_htable (const ast::IAST* var_list, decltype (var_value_) old_htable);

    public:
        explicit Interpreter (std::string& filename) : parser_ (filename.c_str (), parser::INTERPRETER) {}
        ~Interpreter () {}
        Interpreter (const Interpreter& that) = default;
        Interpreter (Interpreter&& that)      = default;
        Interpreter& operator= (const Interpreter& that) = default;
        Interpreter& operator= (Interpreter&& that)      = default;

        lexem::val_t run ();

    };

}

#endif
