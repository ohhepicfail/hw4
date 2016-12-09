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
        EOL,
        END,
        NAP
    };

    const char* string_eq (Operator op);
}

#endif