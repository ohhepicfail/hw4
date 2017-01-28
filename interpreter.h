#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include <unordered_map>

namespace ipr {

    class Interpreter {
    private:
        ast::IAST* root_;
        std::unordered_map <std::string, double> htable_;

        void   calculate_assign (const ast::IAST* assign);
        double calculate_tern   (const ast::IAST* tern);
        bool   calculate_cond   (const ast::IAST* cond);
        double calculate_val    (const ast::IAST* val_root);

    public:
        explicit Interpreter (ast::IAST* prog) : root_ (prog) {}
        ~Interpreter () { delete root_; }
        Interpreter (const Interpreter& that) : root_ (that.root_ ? that.root_->clone () : 0)
                                              , htable_ (that.htable_) {}
        Interpreter (Interpreter&& that) : root_ (that.root_)
                                         , htable_ (std::move (that.htable_)) 
                                         { that.root_ = nullptr; }
        Interpreter& operator= (const Interpreter& that);
        Interpreter& operator= (Interpreter&& that);

        double run ();

    };

}

#endif