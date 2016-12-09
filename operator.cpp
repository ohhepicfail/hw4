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
            case EOL    : return "EOL";     break;
            case END    : return "END";     break;
            case NAP    : return "NAP";     break;
            default     : return "DEFAULT"; break;
        }
    }
}