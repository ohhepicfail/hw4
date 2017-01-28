#include "lexer.h"
#include <cassert>

namespace lexem {

    Lexer::Lexer (const char* filename) {
        auto f = fopen (filename, "rb");
        assert (f);

        fseek (f, 0, SEEK_END);
        unsigned long sz = ftell (f);
        fseek (f, 0, SEEK_SET);

        text_ = new char[sz];
        tsize_ = sz;

        assert (sz == fread (text_, sizeof (char), sz, f));

        fclose (f);

        next_lexem ();
    }


    Lexer::~Lexer () {
        delete[] text_;
    }


    Lexer::Lexer (const Lexer& that) {
        lexem_ = that.cur_lexem ();
        if (that.text_) {
            text_ = new char[that.tsize_];
            std::copy (that.text_, that.text_ + that.tsize_, text_);
            cur_pos_ = that.cur_pos_;
            tsize_   = that.tsize_;
            pos_     = that.pos_;
            line_    = that.line_;
        }
    }


    Lexer::Lexer (Lexer&& that) : lexem_ (that.lexem_)
                                , text_ (that.text_)
                                , cur_pos_ (that.cur_pos_)
                                , tsize_ (that.tsize_)
                                , line_ (that.line_)
                                , pos_ (that.pos_) 
    {
        that.text_    = nullptr;
    }


    Lexer& Lexer::operator= (const Lexer& that) {
        if (this != &that) {
            delete[] text_;
            text_  = nullptr;

            Lexer tmp (that);
            *this = std::move (tmp);
        }
        return *this;
    }


    Lexer& Lexer::operator= (Lexer&& that) {
        delete[] text_;
        lexem_ = that.lexem_;
        text_  = that.text_;
        that.text_  = nullptr;

        cur_pos_ = that.cur_pos_;
        tsize_   = that.tsize_;
        line_    = that.line_;
        pos_     = that.pos_;

        return *this;
    }


    Lexem Lexer::cur_lexem () const {
        return lexem_;
    }


    void Lexer::next_lexem () {
        skip_spaces ();
        if (cur_pos_ < tsize_ && isalpha (text_[cur_pos_]))
            set_var_lexem ();
        else if (cur_pos_ < tsize_ && (isdigit (text_[cur_pos_]) || text_[cur_pos_] == '.'))
            set_val_lexem ();
        else
            set_op_lexem ();

    }


    void Lexer::skip_spaces () {
        while (cur_pos_ < tsize_ && (text_[cur_pos_] == ' ' || text_[cur_pos_] == '\n')) {
            if (text_[cur_pos_] == '\n')
                increase_line ();
            cur_pos_++;
        }
    }


    void Lexer::set_op_lexem () {

        using namespace op;
        Operator op = NAP;
        if (cur_pos_ >= tsize_)
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
                default   :                break;
            }

            cur_pos_++;
            if (c == '>' || c == '<' || c == '!' || c == '=') {
                if (cur_pos_ < tsize_) {
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


    void Lexer::set_val_lexem () {
        double val = .0;

        while (isdigit (text_[cur_pos_])) 
            val = val * 10 + text_[cur_pos_++] - '0';

        if (text_[cur_pos_] == '.') {
            cur_pos_++;

            unsigned counter = 10;
            if (!isdigit (text_[cur_pos_])) {
                printf ("\nincorrect digit %lg. at line %u, pos %u\n\n", val, line_, pos_);
                abort ();
            }
            while (isdigit (text_[cur_pos_])) {
                val += static_cast<double> (text_[cur_pos_++] - '0') / counter;
                counter *= 10;
            }
        }
        lexem_ = Lexem (val, line_, pos_);
        increase_pos ();
    }


    void Lexer::set_var_lexem () {
        auto begin = cur_pos_;
        auto end = begin;
        while (isalpha (text_[end]))
            end++;
        end++;

        char* tmp = new char[end - begin] ();
        std::copy (text_ + begin, text_ + end - 1, tmp);
        cur_pos_ += end - begin - 1;

        lexem_ = Lexem (tmp, line_, pos_);
        delete[] tmp;
        increase_pos ();
    }


}