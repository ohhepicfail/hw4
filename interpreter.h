#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include <unordered_map>

namespace ipr {

    class Interpreter {
    private:
        ast::IAST* root_;
        std::unordered_map <std::string, double> htable_;

        void calculate_assign (const ast::IAST* assign);
        double calculate_val (const ast::IAST* val_root);

    public:
        Interpreter (ast::IAST* prog) : root_ (prog) {}
        ~Interpreter () { delete root_; }
        double run ();

    };

}

#endif