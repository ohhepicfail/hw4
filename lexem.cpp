#include "lexem.h"


namespace lexem {

    Lexem::Lexem (char* var  , unsigned line, unsigned pos) : type_ (VAR)
                                                            , line_ (line)
                                                            , pos_ (pos) 
    { 
        auto n = strlen (var); 
        n++;
        var_  = new char [n];  
        std::copy (var, var + n, var_);
    }

    Lexem::Lexem (const Lexem& that) : type_ (that.type_)
                                     , line_ (that.line_)
                                     , pos_  (that.pos_ ) 
    {
        if (type_ == OP)
            op_ = that.op_;
        else if (type_ == VAL)
            val_ = that.val_;
        else {
            if (that.var_) {
                auto n = strlen (that.var_); 
                n++;
                var_  = new char [n];  
                std::copy (that.var_, that.var_ + n, var_); 
            }
            else 
                var_ = nullptr;
        }
    }


    Lexem::Lexem (Lexem&& that) : type_ (that.type_)
                                , line_ (that.line_)
                                , pos_  (that.pos_ )  
    {
        if (type_ == type::OP)
            op_ = that.op_;
        else if (type_ == type::VAL)
            val_ = that.val_;
        else {
            var_ = that.var_;
            that.var_ = nullptr;
        }
    }


    Lexem& Lexem::operator= (const Lexem& that) {
        if (this != &that) {
            if (type_ == VAR)
                delete[] var_;

            Lexem tmp (that);
            *this = std::move (tmp);
        }

        return *this;
    }


    Lexem& Lexem::operator= (Lexem&& that) {
        if (type_ == VAR)
            delete[] var_;

        type_ = that.type_;
        if (type_ == type::OP)
            op_ = that.op_;
        else if (type_ == type::VAL)
            val_ = that.val_;
        else {
            var_ = that.var_;
            that.var_ = nullptr;
        }

        line_ = that.line_;
        pos_  = that.pos_;

        return *this;
    }
}