#ifndef LEXEM_H
#define LEXEM_H


#include "operator.h"
#include <cassert>
#include <cstring>
#include <algorithm>

namespace lexem {

    enum Lexem_type {
        NAT,
        VAR,
        VAL,
        OP
    };


    class ILexem {
    protected:
        Lexem_type cur_type_ = NAT;  
    public:
        ILexem (Lexem_type t) : cur_type_ (t) {}
        virtual ~ILexem () {}
        virtual ILexem* clone () = 0;

        virtual double       get_val () const = 0;
        virtual const char*  get_var () const = 0;
        virtual op::Operator get_op  () const = 0;
        Lexem_type           get_type () const { return cur_type_; }
    };


    class Val_lexem final: public ILexem {
    private:
        double val_;
    public:
        Val_lexem (double val) : ILexem (VAL), val_ (val) {}
        ~Val_lexem () override {}
        Val_lexem (const Val_lexem& that) : ILexem (VAL), val_ (that.val_) {}
        ILexem* clone () override { return new Val_lexem (*this); }

        double       get_val () const override { return val_; }
        const char*  get_var () const override { assert (0); }
        op::Operator get_op  () const override { assert (0); }
    };


    class Var_lexem final: public ILexem {
    private:
        char* var_ = nullptr;
    public:
        Var_lexem (const char* var);
        ~Var_lexem () override { delete[] var_;}
        Var_lexem (const Var_lexem& that);
        ILexem* clone () override { return new Var_lexem (*this); }

        double       get_val () const override { assert (0); }
        const char*  get_var () const override { return var_; }
        op::Operator get_op  () const override { assert (0); }
        
    };


    class Op_lexem final: public ILexem {
    private:
        op::Operator op_ = op::NAP;
    public:
        Op_lexem (op::Operator op) : ILexem (OP), op_ (op) {}
        ~Op_lexem () override {}
        Op_lexem (const Op_lexem& that) : ILexem (OP), op_ (that.op_) {}
        ILexem* clone () override { return new Op_lexem (*this); }

        double       get_val () const override { assert (0); }
        const char*  get_var () const override { assert (0); }
        op::Operator get_op  () const override { return op_; }
    };

}

#endif