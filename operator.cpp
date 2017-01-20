#include "operator.h"

namespace op {
    const char* string_eq (Operator op) {
        switch (op) {
            case ASSIGN : return "=";       break;
            case ADD    : return "+";       break;
            case SUB    : return "-";       break;
            case MUL    : return "*";       break;
            case DIV    : return "/";       break;
            case CODE   : return "CODE";    break;
            case END    : return "END";     break;
            case OBRT   : return "(";       break;
            case CBRT   : return ")";       break;
            case SMCN   : return ";";       break;
            case NAP    : return "NAP";     break;
            default     : return "DEFAULT"; break;
        }
    }
}
