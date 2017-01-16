#ifndef AST_H
#define AST_H


#include <cstring>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include "operator.h"
#include "type.h"

using namespace type;

namespace ast {


    class IAST {
    public:
        IAST () {}
        virtual ~IAST () {};
        virtual IAST* clone () const = 0;

        void print (const char* filename) const;
        virtual void dprint (FILE* f) const = 0;

        virtual const IAST* get_left  () const = 0;
        virtual const IAST* get_right () const = 0;

        virtual double       get_val  () const = 0;
        virtual const char*  get_var  () const = 0;
        virtual op::Operator get_op   () const = 0;
        virtual Type         get_type () const { return NAT; }
    };


    class Val_AST final: public IAST {
    private:
        double val_;
    public:
        Val_AST (double val) : val_ (val) {}
        ~Val_AST () override {}
        Val_AST (const Val_AST& that) : val_ (that.val_) {}
        IAST* clone () const override { return new Val_AST (*this); }

        void dprint (FILE* f) const override;

        const IAST* get_left  () const override { assert (0); }
        const IAST* get_right () const override { assert (0); }

        double       get_val  () const override { return val_; }
        const char*  get_var  () const override { assert (0); }
        op::Operator get_op   () const override { assert (0); }
        Type         get_type () const override { return VAL; }
    };


    class Var_AST final: public IAST {
    private:
        char* var_ = nullptr;
    public:
        explicit Var_AST (const char* var);
        ~Var_AST () override { delete[] var_; }
        Var_AST (const Var_AST& that);
        IAST* clone () const override { return new Var_AST (*this); } 

        void dprint (FILE* f) const override;

        const IAST* get_left  () const override { assert (0); }
        const IAST* get_right () const override { assert (0); }

        double       get_val  () const override { assert (0); }
        const char*  get_var  () const override { return var_; }
        op::Operator get_op   () const override { assert (0); }
        Type         get_type () const override { return VAR; }
    };


    class Op_AST final: public IAST {
    private:
        op::Operator op_ = op::NAP;
        IAST* left_  = nullptr;
        IAST* right_ = nullptr;
    public:
        explicit Op_AST (op::Operator op, IAST* left, IAST* right) : op_ (op), left_ (left), right_ (right) {}
        ~Op_AST () override { delete left_; delete right_; }
        Op_AST (const Op_AST& that) : op_ (that.op_) {
            if (that.left_)
                left_ = that.left_->clone ();
            if (that.right_)
                right_ = that.right_->clone ();
        }
        IAST* clone () const override { return new Op_AST (*this); }

        void dprint (FILE* f) const override;

        const IAST* get_left  () const override { return left_;  }
        const IAST* get_right () const override { return right_; }

        double       get_val  () const override { assert (0); }
        const char*  get_var  () const override { assert (0); }
        op::Operator get_op   () const override { return op_; }
        Type         get_type () const override { return OP; }
    };
}


#endif
