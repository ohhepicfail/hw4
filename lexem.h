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
        ILexem (unsigned line, unsigned pos) : line_ (line), pos_ (pos) {}
        virtual ~ILexem () {}
        virtual ILexem* clone () const = 0;

        virtual double       get_val  () const = 0;
        virtual const char*  get_var  () const = 0;
        virtual op::Operator get_op   () const = 0;
        virtual Type         get_type () const { return NAT; }

        unsigned get_line () const { return line_; }
        unsigned get_pos  () const { return pos_;  }


    private:
        unsigned line_;
        unsigned pos_;
    };


    class Val_lexem final: public ILexem {
    private:
        double val_;
    public:
        Val_lexem (double val, unsigned line, unsigned pos) : ILexem (line, pos), val_ (val) {}
        ~Val_lexem () override {}
        Val_lexem (const Val_lexem& that) : ILexem (that.get_line (), that.get_pos ()), val_ (that.val_) {}
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
        Var_lexem (const char* var, unsigned line, unsigned pos);
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
        Op_lexem (op::Operator op, unsigned line, unsigned pos) : ILexem (line, pos), op_ (op) {}
        ~Op_lexem () override {}
        Op_lexem (const Op_lexem& that) : ILexem (that.get_line (), that.get_pos ()), op_ (that.op_) {}
        ILexem* clone () const override { return new Op_lexem (*this); }

        double       get_val  () const override { assert (0); }
        const char*  get_var  () const override { assert (0); }
        op::Operator get_op   () const override { return op_; }
        Type         get_type () const override { return OP; }
    };

}

#endif