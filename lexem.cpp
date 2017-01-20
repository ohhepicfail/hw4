#include "lexem.h"


namespace lexem {

    Var_lexem::Var_lexem (const char* var, unsigned line, unsigned pos) : ILexem (line, pos) { 
        auto n = strlen (var); 
        n++;
        var_  = new char [n];  
        std::copy (var, var + n, var_);
    }


    Var_lexem::Var_lexem (const Var_lexem& that)
        : ILexem (that.get_line (), that.get_pos ()) {
        if (that.var_) {
            auto n = strlen (that.var_); 
            n++;
            var_  = new char [n];  
            std::copy (that.var_, that.var_ + n, var_);
        }
    }

}