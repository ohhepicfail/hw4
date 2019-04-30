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
        ENDWHILE,
        ENDIF,
        ENDFUNC,
        ENDFALSE,
        ENDTRUE,
        ENDCOND,
        CAPTURE,
        COMMA,
        WHILE,
        OBRACE,
        CBRACE,
        FUNCTION,
        CALL,
        SOBRT,
        SCBRT,
        DEF,
        AACCESS,
        NAP
    };

    const char* string_eq (Operator op);
    const char* asm_string (Operator op);
}

#endif
