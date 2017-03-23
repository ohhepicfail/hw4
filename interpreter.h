#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"
#include <stack>
#include <unordered_map>

namespace ipr {

    class Interpreter {
    private:
        parser::Parser parser_;
        // std::stack <const ast::IAST*> prog_nodes_;
        std::unordered_map <std::string, double> var_value_;

        void   calculate ();
        double calculate_val  (const ast::IAST* val_root);
        bool   calculate_cond (const ast::IAST* cond);

        double get_value (const ast::IAST* node);

        void create_htable (const ast::IAST* var_list);
        void update_htable (const ast::IAST* var_list, decltype (var_value_) old_htable);

    public:
        explicit Interpreter (std::string& filename) : parser_ (filename.c_str (), parser::INTERPRETER) {}
        ~Interpreter () {}
        // Interpreter (const Interpreter& that) : root_ (that.root_ ? that.root_->clone () : 0)
        //                                       , prog_nodes_ (that.prog_nodes_)
        //                                       , var_value_ (that.var_value_) {}
        // Interpreter (Interpreter&& that) : root_ (that.root_)
        //                                  , prog_nodes_ (std::move (that.prog_nodes_))
        //                                  , var_value_ (std::move (that.var_value_)) 
        //                                  { that.root_ = nullptr; }
        // Interpreter& operator= (const Interpreter& that);
        // Interpreter& operator= (Interpreter&& that);

        double run ();

    };

}

#endif