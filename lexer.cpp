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
        delete lexem_;
        delete[] text_;
    }


    Lexer::Lexer (const Lexer& that) {
        if (that.lexem_)
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
        that.lexem_   = nullptr;
        that.text_    = nullptr;
        that.cur_pos_ = 0;
        that.tsize_   = 0;
        that.line_    = 1;
        that.pos_     = 1;
    }


    Lexer& Lexer::operator= (const Lexer& that) {
        if (this != &that) {
            delete lexem_;
            delete[] text_;
            lexem_ = nullptr;
            text_  = nullptr;

            Lexer tmp (that);
            *this = std::move (tmp);
        }
        return *this;
    }


    Lexer& Lexer::operator= (Lexer&& that) {
        delete lexem_;
        delete[] text_;
        lexem_ = that.lexem_;
        text_  = that.text_;
        that.lexem_ = nullptr;
        that.text_  = nullptr;

        cur_pos_ = that.cur_pos_;
        tsize_   = that.tsize_;
        line_    = that.line_;
        pos_     = that.pos_;
        that.cur_pos_ = 0;
        that.tsize_   = 0;
        that.line_    = 1;
        that.pos_     = 1;

        return *this;
    }


    ILexem* Lexer::cur_lexem () const {
        ILexem* tmp = lexem_->clone ();

        return tmp;
    }


    void Lexer::next_lexem () {
        delete lexem_;

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
                case '+'  : op = ADD;    break;
                case '-'  : op = SUB;    break;
                case '*'  : op = MUL;    break;
                case '/'  : op = DIV;    break;
                case '='  : op = ASSIGN; break;
                case '\0' : op = END;    break;
                case '('  : op = OBRT;   break;
                case ')'  : op = CBRT;   break;
                case ';'  : op = SMCN;   break;
                default   :              break;
            }

            cur_pos_++;
            if (op == NAP) {
                printf ("\nunrecognized lexem \'%c\' at line %u, pos %u\n\n", c, line_, pos_);
                abort ();
            }
        }
        lexem_ = new Op_lexem (op, line_, pos_);
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
        lexem_ = new Val_lexem (val, line_, pos_);
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

        lexem_ = new Var_lexem (tmp, line_, pos_);
        delete[] tmp;
        increase_pos ();
    }


}