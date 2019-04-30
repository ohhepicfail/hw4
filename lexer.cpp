#include "lexer.h"
#include <cassert>
#include <fstream>
#include <sstream>
#include <cstring>
#include <iostream>
#include <sstream>

namespace lexem {

    Lexer::Lexer (const char* filename) {
        std::ifstream ifs (filename);
        if (!ifs.is_open ()) {
            printf ("lexer can't open the file\n");
            abort ();
        }

        std::stringstream buffer;
        buffer << ifs.rdbuf ();
        text_ = buffer.str ();    

        ifs.close ();

        next_lexem ();
    }


    Lexer::Lexer (const Lexer& that) {
        lexem_   = that.get_cur_lexem ();
        text_    = that.text_;
        cur_pos_ = that.cur_pos_;
        pos_     = that.pos_;
        line_    = that.line_;
    }


    Lexer::Lexer (Lexer&& that) : lexem_ (that.lexem_)
                                , text_ (std::move (that.text_))
                                , cur_pos_ (that.cur_pos_)
                                , line_ (that.line_)
                                , pos_ (that.pos_) 
    {}


    Lexer& Lexer::operator= (const Lexer& that) {
        if (this != &that) {
            Lexer tmp (that);
            *this = std::move (tmp);
        }
        return *this;
    }


    Lexer& Lexer::operator= (Lexer&& that) {
        lexem_   = that.lexem_;
        text_    = std::move (that.text_);
        cur_pos_ = that.cur_pos_;
        line_    = that.line_;
        pos_     = that.pos_;

        return *this;
    }


    Lexem Lexer::get_cur_lexem () const {
        return lexem_;
    }


    void Lexer::next_lexem () {
        skip_spaces ();
        if (cur_pos_ < text_.length () && isalpha (text_[cur_pos_]))
            set_var_lexem ();
        else if (cur_pos_ < text_.length () && (isdigit (text_[cur_pos_]) || text_[cur_pos_] == '.'))
            set_val_lexem ();
        else
            set_op_lexem ();

    }


    void Lexer::skip_spaces () {
        while (cur_pos_ < text_.length () && (text_[cur_pos_] == ' ' || text_[cur_pos_] == '\n' || text_[cur_pos_] == '\t')) {
            if (text_[cur_pos_] == '\n')
                increase_line ();
            cur_pos_++;
        }
    }


    void Lexer::set_op_lexem () {
        using namespace op;
        Operator op = NAP;
        if (cur_pos_ >= text_.length ())
            op = END;
        else {
            auto c = text_[cur_pos_];

            switch (c) {
                case '+'  : op = ADD;      break;
                case '-'  : op = SUB;      break;
                case '*'  : op = MUL;      break;
                case '/'  : op = DIV;      break;
                case '='  : op = ASSIGN;   break;
                case '\0' : op = END;      break;
                case '('  : op = OBRT;     break;
                case ')'  : op = CBRT;     break;
                case ';'  : op = SMCN;     break;
                case '>'  : op = MORE;     break;
                case '<'  : op = LESS;     break;
                case '?'  : op = QUESTION; break;
                case '!'  : op = NOTEQUAL; break;
                case ':'  : op = COLON;    break;
                case ','  : op = COMMA;    break;
                case '{'  : op = OBRACE;   break;
                case '}'  : op = CBRACE;   break;
                case '['  : op = SOBRT;    break;
                case ']'  : op = SCBRT;    break;
                default   :                break;
            }

            cur_pos_++;
            if (c == '>' || c == '<' || c == '!' || c == '=') {
                if (cur_pos_ < text_.length ()) {
                    auto next_c = text_[cur_pos_];
                    if (next_c == '=') {
                        cur_pos_++;
                        switch (c) {
                            default  :                break;
                            case '>' : op = MOREOREQ; break;
                            case '<' : op = LESSOREQ; break;
                            case '=' : op = EQUAL;    break;
                            case '!' : op = NOTEQUAL; break;
                        }
                    }
                    else if (c == '!')
                        op = NAP;
                }
            }

            if (op == NAP) {
                printf ("\nunrecognized lexem \'%c\' at line %u, pos %u\n\n", c, line_, pos_);
                abort ();
            }
        }
        lexem_ = Lexem (op, line_, pos_);
        increase_pos ();
    }


    void Lexer::set_var_lexem () {
        auto begin = cur_pos_;
        auto end = begin;
        while (isalpha (text_[end]))
            end++;
        end++;

        std::string tmp;
        tmp.reserve (end - begin);
        tmp.assign (text_, begin, end - begin - 1);
        cur_pos_ += end - begin - 1;

        if (tmp == string_eq (IF))
            lexem_ = Lexem (IF, line_, pos_);
        else if (tmp == string_eq (ENDIF))
            lexem_ = Lexem (ENDIF, line_, pos_);
        else if (tmp == string_eq (CAPTURE))
            lexem_ = Lexem (CAPTURE, line_, pos_);
        else if (tmp == string_eq (WHILE))
            lexem_ = Lexem (WHILE, line_, pos_);
        else if (tmp == string_eq (FUNCTION))
            lexem_ = Lexem (FUNCTION, line_, pos_);
        else
            lexem_ = Lexem (tmp.c_str (), line_, pos_); 

        increase_pos ();
    }

    void Lexer::set_val_lexem () {
      lexem::val_t val = 0;

        while (isdigit (text_[cur_pos_]))
            val = val * 10 + text_[cur_pos_++] - '0';

        if (!std::numeric_limits<val_t>::is_integer && text_[cur_pos_] == '.') {
            cur_pos_++;

            unsigned counter = 10;
            if (!isdigit (text_[cur_pos_])) {
                std::stringstream ss;
                ss << val;
                printf ("\nincorrect digit %s. at line %u, pos %u\n\n", ss.str().c_str(), line_, pos_);
                abort ();
            }
            while (isdigit (text_[cur_pos_])) {
                val += static_cast<lexem::val_t> (text_[cur_pos_++] - '0') / counter;
                counter *= 10;
            }
        }
        lexem_ = Lexem (val, line_, pos_);
        increase_pos ();
    }
}
