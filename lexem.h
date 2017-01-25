#ifndef LEXEM_H
#define LEXEM_H


#include "operator.h"
#include "type.h"
#include <cassert>
#include <cstring>
#include <algorithm>

namespace lexem {
    using namespace type;
    using namespace op;

    class Lexem {
    public:
        Lexem () : type_ (NAT), line_ (0), pos_ (0) {}
        Lexem (Operator op, unsigned line, unsigned pos) : type_ (OP) , op_ (op)  , line_ (line), pos_ (pos) {}
        Lexem (double val , unsigned line, unsigned pos) : type_ (VAL), val_ (val), line_ (line), pos_ (pos) {}
        Lexem (char* var  , unsigned line, unsigned pos);
        ~Lexem () { if (type_ == VAR) delete[] var_; }
        Lexem (const Lexem& that);
        Lexem (Lexem&& that);
        Lexem& operator= (const Lexem& that);
        Lexem& operator= (Lexem&& that);

        unsigned get_line () const { return line_; }
        unsigned get_pos  () const { return pos_ ; }

        Type get_type () const { return type_; }
        double get_val () const {
            if (type_ != VAL) {
                printf ("\nError in Lexem. I can't return val, because type != VAL\n"); 
                abort ();
            } 
            else 
                return val_; 
        }
        Operator get_op () const {
            if (type_ != OP) {
                printf ("\nError in Lexem. I can't return op, because type != OP\n"); 
                abort ();
            } 
            else 
                return op_; 
        }
        const char* get_var () const {
            if (type_ != VAR) {
                printf ("\nError in Lexem. I can't return var, because type != VAR\n"); 
                abort ();
            } 
            else 
                return var_; 
        }

    private:
        Type type_;
        union {
            Operator op_;
            double val_;
            char*  var_;
        };

        unsigned line_;
        unsigned pos_;
    };
}

#endif