#ifndef OPERATOR_H
#define OPERATOR_H

namespace op {

    enum Operator {
        ASSIGN,
        ADD,
        SUB,
        MUL,
        DIV,
        CODE,
        TERN,
        END,
        OBRT,
        CBRT,
        SMCN,
        QUESTION,
        MORE,
        MOREOREQ,
        LESS,
        LESSOREQ,
        EQUAL,
        NOTEQUAL,
        COLON,
        IF,
        ENDIF,
        CAPTURE,
        COMMA,
        WHILE,
        OBRACE,
        CBRACE,
        FUNCTION,
        NAP
    };

    const char* string_eq (Operator op);
}

#endif