#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include <unordered_map>

namespace ipr {

    class Interpreter {
    private:
        ast::IAST* root_;
        std::unordered_map <std::string, double> htable_;

        void   calculate_code   (const ast::IAST* code);
        void   calculate_while  (const ast::IAST* code);
        void   calculate_if     (const ast::IAST* code);
        void   calculate_assign (const ast::IAST* assign);
        double calculate_tern   (const ast::IAST* tern);
        bool   calculate_cond   (const ast::IAST* cond);
        double calculate_val    (const ast::IAST* val_root);

        void update_htable    (const ast::IAST* var_list, decltype (htable_) old_htable);
        void create_if_htable (const ast::IAST* var_list);

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