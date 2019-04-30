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
            case ENDWHILE   : return "endwhile";break;
            case ENDFUNC    : return "endfunc"; break;
            case ENDTRUE    : return "endtrue"; break;
            case ENDFALSE   : return "endfalse";
            case ENDCOND    : return "endcond"; break;
            case CAPTURE    : return "capture"; break;
            case COMMA      : return ",";       break;
            case WHILE      : return "while";   break;
            case OBRACE     : return "{";       break;
            case CBRACE     : return "}";       break;
            case FUNCTION   : return "function";break;
            case CALL       : return "call";    break;
            case SOBRT      : return "[";       break;
            case SCBRT      : return "]";       break;
            case DEF        : return "def";     break;
            case AACCESS    : return "aaccess"; break;
            case NAP        : return "NAP";     break;
            default         : return "DEFAULT"; break;
        }
    }

    const char* asm_string (Operator op) {
        switch (op) {
            case ADD        : return "\tadd t2, t0, t1\n";    break;
            case SUB        : return "\tsub t2, t0, t1\n";    break;
            case MUL        : return "\tmul t2, t0, t1\n";    break;
            case DIV        : return "\tdiv t2, t0, t1\n";    break;
            case MORE       : return "\tsub t2, t0, t1\n"
                                     "\tsgtz t2, t2\n";       break;
            case MOREOREQ   : return "\tsub t2, t0, t1\n"
                                     "\tseqz t1, t2\n"
                                     "\tsgtz t0, t2\n"
                                     "\tor t2, t0, t1\n";     break;
            case LESS       : return "\tsub t2, t0, t1\n"
                                     "\tsltz t2, t2\n";       break;
            case LESSOREQ   : return "\tsub t2, t0, t1\n"
                                     "\tseqz t1, t2\n"
                                     "\tsltz t0, t2\n"
                                     "\tor t2, t0, t1\n";     break;
            case EQUAL      : return "\tsub t2, t0, t1\n"
                                     "\tseqz t2, t2\n";       break;
            case NOTEQUAL   : return "\tsub t2, t0, t1\n"
                                     "\tsnez t2, t2\n";       break;
            default         : return "DEFAULT\n";           break; 
        }
    }
}
