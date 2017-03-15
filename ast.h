#ifndef AST_H
#define AST_H


#include <cstring>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <string>
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

        virtual double              get_val  () const { printf ("Can't return val. I'm IAST\n"); abort (); }
        virtual const std::string&  get_var  () const { printf ("Can't return var. I'm IAST\n"); abort (); }
        virtual op::Operator        get_op   () const { printf ("Can't return  op. I'm IAST\n"); abort (); }
        virtual Type                get_type () const { return NAT; }
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

        const IAST* get_left  () const override { return nullptr;}
        const IAST* get_right () const override { return nullptr; }

        double       get_val  () const override { return val_; }
        Type         get_type () const override { return VAL; }
    };


    class Var_AST final: public IAST {
    private:
        std::string var_;
    public:
        explicit Var_AST (const char* var)        : var_ (var) {}
        explicit Var_AST (const std::string& var) : var_ (var) {}
        ~Var_AST () override {}
        Var_AST (const Var_AST& that) : var_ (that.var_) {}
        IAST* clone () const override { return new Var_AST (*this); } 

        void dprint (FILE* f) const override;

        const IAST* get_left  () const override { return nullptr; }
        const IAST* get_right () const override { return nullptr; }

        const std::string&  get_var  () const override { return var_; }
        Type                get_type () const override { return VAR; }
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

        op::Operator get_op   () const override { return op_; }
        Type         get_type () const override { return OP; }
    };
}


#endif
