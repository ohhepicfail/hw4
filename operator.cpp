#include "operator.h"

namespace op {
    const char* string_eq (Operator op) {
        switch (op) {
            case ASSIGN     : return "=";       break;
            case ADD        : return "+";       break;
            case SUB        : return "-";       break;
            case MUL        : return "*";       break;
            case DIV        : return "/";       break;
            case CODE       : return "CODE";    break;
            case TERN       : return "TERN";    break;
            case END        : return "END";     break;
            case OBRT       : return "(";       break;
            case CBRT       : return ")";       break;
            case SMCN       : return ";";       break;
            case QUESTION   : return "?";       break;
            case MORE       : return ">";       break;
            case MOREOREQ   : return ">=";      break;
            case LESS       : return "<";       break;
            case LESSOREQ   : return "<=";      break;
            case EQUAL      : return "==";      break;
            case NOTEQUAL   : return "!=";      break;
            case COLON      : return ":";       break;
            case IF         : return "if";      break;
            case ENDIF      : return "endif";   break;
            case CAPTURE    : return "capture"; break;
            case COMMA      : return ",";       break;
            case WHILE      : return "while";   break;
            case OBRACE     : return "{";       break;
            case CBRACE     : return "}";       break;
            case NAP        : return "NAP";     break;
            default         : return "DEFAULT"; break;
        }
    }
}
