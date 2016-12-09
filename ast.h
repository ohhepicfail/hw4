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
        virtual IAST* clone () = 0;
    };


    class Val_AST final: public IAST {
    private:
        double val_;
    public:
        Val_AST (double val) : val_ (val) {}
        ~Val_AST () override {}
        Val_AST (const Val_AST& that) : val_ (that.val_) {}
        IAST* clone () override { return new Val_AST (*this); }

        void dprint (FILE* f) const override;
    };


    class Var_AST final: public IAST {
    private:
        char* var_ = nullptr;
    public:
        Var_AST (const char* var);
        ~Var_AST () override { delete[] var_; }
        Var_AST (const Var_AST& that);
        IAST* clone () override { return new Var_AST (*this); } 

        void dprint (FILE* f) const override;
    };


    class Op_AST final: public IAST {
    private:
        op::Operator op_ = op::NAP;
        IAST* left_  = nullptr;
        IAST* right_ = nullptr;
    public:
        Op_AST (op::Operator op, IAST* left, IAST* right) : op_ (op), left_ (left), right_ (right) {}
        ~Op_AST () override { delete left_; delete right_; }
        Op_AST (const Op_AST& that) : op_ (that.op_) {
            if (that.left_)
                left_ = that.left_->clone ();
            if (that.right_)
                right_ = that.right_->clone ();
        }
        IAST* clone () override { return new Op_AST (*this); }

        void dprint (FILE* f) const override;
    };
}


#endif