#ifndef LEXEM_H
#define LEXEM_H


#include "operator.h"
#include "type.h"
#include <cassert>
#include <cstring>
#include <algorithm>

namespace lexem {
    using namespace type;

    class ILexem { 
    public:
        ILexem () {}
        virtual ~ILexem () {}
        virtual ILexem* clone () const = 0;

        virtual double       get_val  () const = 0;
        virtual const char*  get_var  () const = 0;
        virtual op::Operator get_op   () const = 0;
        virtual Type         get_type () const { return NAT; }
    };


    class Val_lexem final: public ILexem {
    private:
        double val_;
    public:
        Val_lexem (double val) : val_ (val) {}
        ~Val_lexem () override {}
        Val_lexem (const Val_lexem& that) : val_ (that.val_) {}
        ILexem* clone () const override { return new Val_lexem (*this); }

        double       get_val  () const override { return val_; }
        const char*  get_var  () const override { assert (0); }
        op::Operator get_op   () const override { assert (0); }
        Type         get_type () const override { return VAL; }
    };


    class Var_lexem final: public ILexem {
    private:
        char* var_ = nullptr;
    public:
        explicit Var_lexem (const char* var);
        ~Var_lexem () override { delete[] var_;}
        Var_lexem (const Var_lexem& that);
        ILexem* clone () const override { return new Var_lexem (*this); }

        double       get_val  () const override { assert (0); }
        const char*  get_var  () const override { return var_; }
        op::Operator get_op   () const override { assert (0); }
        Type         get_type () const override { return VAR; }
        
    };


    class Op_lexem final: public ILexem {
    private:
        op::Operator op_ = op::NAP;
    public:
        explicit Op_lexem (op::Operator op) : op_ (op) {}
        ~Op_lexem () override {}
        Op_lexem (const Op_lexem& that) : op_ (that.op_) {}
        ILexem* clone () const override { return new Op_lexem (*this); }

        double       get_val  () const override { assert (0); }
        const char*  get_var  () const override { assert (0); }
        op::Operator get_op   () const override { return op_; }
        Type         get_type () const override { return OP; }
    };

}

#endif