#ifndef LEXER_H
#define LEXER_H

#include "lexem.h"
#include <cstdio>
#include <cctype>

namespace lexem {

    class Lexer {
    private:
        ILexem* lexem_ = nullptr;
        char* text_ = nullptr;
        unsigned cur_pos_ = 0;
        unsigned tsize_ = 0;

        unsigned line_ = 1;
        unsigned pos_ = 1;

        void increase_line () { line_++; pos_ = 1; }
        void increase_pos  () { pos_++; }

        void skip_spaces ();
        void set_var_lexem ();
        void set_val_lexem ();
        void set_op_lexem ();
    public:
        explicit Lexer (const char* filename);
        ~Lexer ();
        Lexer (const Lexer& that);
        Lexer (Lexer&& that);
        Lexer& operator= (const Lexer& that);
        Lexer& operator= (Lexer&& that);

        ILexem* cur_lexem () const;
        void next_lexem ();
    };


}


#endif
