#ifndef LEXER_H
#define LEXER_H

#include "lexem.h"
#include <cstdio>

namespace lexem {

    class Lexer {
    private:
        ILexem* lexem_ = nullptr;
        char* text_ = nullptr;
        unsigned cur_pos_ = 0;
        unsigned tsize_ = 0;

        void skip_spaces ();
        void set_var_lexem ();
        void set_val_lexem ();
        void set_op_lexem ();
    public:
        Lexer (const char* filename);
        ~Lexer ();
        Lexer (const Lexer& that);
        Lexer (Lexer&& that);
        Lexer& operator= (const Lexer& that);
        Lexer& operator= (Lexer&& that);

        ILexem* cur_lexem () const;
        ILexem* next_lexem ();
    };


}


#endif