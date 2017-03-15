#include "lexem.h"


namespace lexem {
    Lexem::Lexem (const Lexem& that) : type_ (that.type_)
                                     , line_ (that.line_)
                                     , pos_  (that.pos_ ) 
    {
        if (type_ == OP)
            op_ = that.op_;
        else if (type_ == VAL)
            val_ = that.val_;
        else 
            var_ = that.var_;
    }


    Lexem::Lexem (Lexem&& that) : type_ (that.type_)
                                , line_ (that.line_)
                                , pos_  (that.pos_ )  
    {
        if (type_ == type::OP)
            op_ = that.op_;
        else if (type_ == type::VAL)
            val_ = that.val_;
        else 
            var_ = std::move (that.var_);
    }


    Lexem& Lexem::operator= (const Lexem& that) {
        if (this != &that) {
            Lexem tmp (that);
            *this = std::move (tmp);
        }

        return *this;
    }


    Lexem& Lexem::operator= (Lexem&& that) {
        type_ = that.type_;
        if (type_ == type::OP)
            op_ = that.op_;
        else if (type_ == type::VAL)
            val_ = that.val_;
        else
            var_ = std::move (that.var_);

        line_ = that.line_;
        pos_  = that.pos_;

        return *this;
    }

    
    bool Lexem::is_closing_operator () {
        if (type_ == type::OP && (op_ == op::END || op_ == op::ENDIF || op_ == op::CBRACE))
            return true;
        return false;
    }   


    bool Lexem::is_semicolon () {
        if (type_ == type::OP && op_ == op::SMCN)
            return true;
        return false;
    }


    bool Lexem::is_while () {
        if (type_ == type::OP && op_ == op::WHILE)
            return true;
        return false;
    }


    bool Lexem::is_open_brace () {
        if (type_ == type::OP && op_ == op::OBRACE)
            return true;
        return false;
    }


    bool Lexem::is_close_brace () {
        if (type_ == type::OP && op_ == op::CBRACE)
            return true;
        return false;
    }

    
    bool Lexem::is_if () {
        if (type_ == type::OP && op_ == op::IF)
            return true;
        return false;
    }


    bool Lexem::is_endif () {
        if (type_ == type::OP && op_ == op::ENDIF)
            return true;
        return false;
    }


    bool Lexem::is_assign () {
        if (type_ == type::OP && op_ == op::ASSIGN)
            return true;
        return false;
    }


    bool Lexem::is_question () {
        if (type_ == type::OP && op_ == op::QUESTION)
            return true;
        return false;
    }


    bool Lexem::is_colon () {
        if (type_ == type::OP && op_ == op::COLON)
            return true;
        return false;
    }


    bool Lexem::is_open_bracket () {
        if (type_ == type::OP && op_ == op::OBRT)
            return true;
        return false;
    }


    bool Lexem::is_close_bracket () {
        if (type_ == type::OP && op_ == op::CBRT)
            return true;
        return false;
    }


    bool Lexem::is_comparison_operator () {
        if (type_ == type::OP && (op_ == op::MORE 
                               || op_ == op::LESS
                               || op_ == op::MOREOREQ 
                               || op_ == op::LESSOREQ
                               || op_ == op::EQUAL 
                               || op_ == op::NOTEQUAL))
            return true;
        return false;
    }


    bool Lexem::is_add () {
        if (type_ == type::OP && op_ == op::ADD)
            return true;
        return false;
    }


    bool Lexem::is_sub () {
        if (type_ == type::OP && op_ == op::SUB)
            return true;
        return false;
    }


    bool Lexem::is_mul () {
        if (type_ == type::OP && op_ == op::MUL)
            return true;
        return false;
    }


    bool Lexem::is_div () {
        if (type_ == type::OP && op_ == op::DIV)
            return true;
        return false;
    }


    bool Lexem::is_capture () {
        if (type_ == type::OP && op_ == op::CAPTURE)
            return true;
        return false;
    }
}