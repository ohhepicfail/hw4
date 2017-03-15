#ifndef LEXEM_H
#define LEXEM_H


#include "operator.h"
#include "type.h"
#include <cassert>
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <string>

namespace lexem {
    using namespace type;
    using namespace op;

    class Lexem {
    public:
        Lexem () : type_ (NAT), var_ (), line_ (0), pos_ (0) {}
        Lexem (Operator op           , unsigned line, unsigned pos) : type_ (OP) , op_ (op)  , line_ (line), pos_ (pos) {}
        Lexem (double val            , unsigned line, unsigned pos) : type_ (VAL), val_ (val), line_ (line), pos_ (pos) {}
        Lexem (const char* var       , unsigned line, unsigned pos) : type_ (VAR), var_ (var), line_ (line), pos_ (pos) {} 
        Lexem (const std::string& var, unsigned line, unsigned pos) : type_ (VAR), var_ (var), line_ (line), pos_ (pos) {} 
        ~Lexem () {}
        Lexem (const Lexem& that);
        Lexem (Lexem&& that);
        Lexem& operator= (const Lexem& that);
        Lexem& operator= (Lexem&& that);

        unsigned get_line () const { return line_; }
        unsigned get_pos  () const { return pos_ ; }

        bool is_closing_operator ();
        bool is_semicolon ();
        bool is_while ();
        bool is_open_brace ();
        bool is_close_brace ();
        bool is_if ();
        bool is_endif ();
        bool is_assign ();
        bool is_question ();
        bool is_colon ();
        bool is_open_bracket ();
        bool is_close_bracket ();
        bool is_comparison_operator ();
        bool is_comma ();
        bool is_add ();
        bool is_sub ();
        bool is_mul ();
        bool is_div ();
        bool is_capture ();

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
        const std::string& get_var () const {
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
        };
        std::string var_;       // there is a trouble with std::string in anonimous union :(

        unsigned line_;
        unsigned pos_;
    };
}

#endif