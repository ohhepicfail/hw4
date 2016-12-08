#ifndef AST_H
#define AST_H


#include <cstring>
#include <algorithm>
#include "operator.h"


namespace ast {


    class IAST {
    public:
        IAST () {}
        virtual ~IAST () {};
        void print (const char* filename);
        virtual void dprint (FILE* f) const = 0;
    };


    class Val_AST final: public IAST {
    private:
        double val_;
    public:
        Val_AST (double val) : val_ (val) {}
        ~Val_AST () override {}
        void dprint (FILE* f) const override;
    };


    class Var_AST final: public IAST {
    private:
        char* var_ = nullptr;
    public:
        Var_AST (const char* var);
        ~Var_AST () override { delete[] var_; } 
        void dprint (FILE* f) const override;
    };


    class Op_AST final: public IAST {
    private:
        Operator op_ = NAP;
        IAST* left_ = nullptr;
        IAST* right_ = nullptr;
    public:
        Op_AST (Operator op, IAST* left, IAST* right) : op_ (op), left_ (left), right_ (right) {}
        ~Op_AST () override { delete left_; delete right_; }
        void dprint (FILE* f) const override;
    };
}


#endif