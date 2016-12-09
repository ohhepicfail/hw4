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

        auto tmp = next_lexem ();
        delete tmp;
    }


    Lexer::~Lexer () {
        delete lexem_;
        delete[] text_;
    }
    // Lexer (const Lexer& that);
    // Lexer (Lexer&& that);
    // Lexer& operator= (const Lexer& that);
    // Lexer& operator= (Lexer&& that);

    ILexem* Lexer::cur_lexem () const {
        ILexem* tmp = lexem_->clone ();

        return tmp;
    }


    ILexem* Lexer::next_lexem () {
        delete lexem_;

        skip_spaces ();

        if (isalpha (text_[cur_pos_]))
            set_var_lexem ();
        else if (isdigit (text_[cur_pos_]) || text_[cur_pos_] == '.')
            set_val_lexem ();
        else
            set_op_lexem ();

        return cur_lexem ();
    }


    void Lexer::skip_spaces () {
        while (text_[cur_pos_] == ' ')
            cur_pos_++;
    }


    void Lexer::set_op_lexem () {

        using namespace op;
        auto c = text_[cur_pos_];
        Operator op = NAP;

        switch (c) {
            case '+'  : op = ADD;    break;
            case '-'  : op = SUB;    break;
            case '*'  : op = MUL;    break;
            case '/'  : op = DIV;    break;
            case '='  : op = ASSIGN; break;
            case '\n' : op = EOL;    break;
            case EOF  : op = END;    break;
        }

        cur_pos_++;
        if (cur_pos_ == tsize_)
            op = END;
        assert (op != NAP);

        lexem_ = new Op_lexem (op);
    }


    void Lexer::set_val_lexem () {
        double val = .0;

        while (isdigit (text_[cur_pos_]))
            val = val * 10 + text_[cur_pos_++] - '0';

        if (text_[cur_pos_] != '.') {
            lexem_ = new Val_lexem (val);
            return;
        }
        cur_pos_++;

        unsigned counter = 10;
        while (isdigit (text_[cur_pos_])) {
            val += static_cast<double> (text_[cur_pos_++] - '0') / counter;
            counter *= 10;
        }

        lexem_ = new Val_lexem (val);
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

        lexem_ = new Var_lexem (tmp);
        delete[] tmp;
    }


}


int main () {
    using namespace lexem;

    Lexer lr ("code.txt");
    ILexem* tmp = lr.cur_lexem ();
    while (1) {
        if (tmp->get_type () == VAR)
            printf ("var\t%s\n", tmp->get_var ());
        if (tmp->get_type () == VAL)
            printf ("val\t%lf\n", tmp->get_val ()); 
        if (tmp->get_type () == OP) {
            auto t = tmp->get_op ();
            printf ("op \t%s\n", op::string_eq (t));
            if (t == op::END)
                break;   
        }
        delete tmp;
        tmp = lr.next_lexem ();
    }

    delete tmp;
    return 0;
}