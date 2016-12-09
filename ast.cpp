#include <cstdio>
#include <cassert>
#include "ast.h"

namespace ast {

/*
 *
 *
 *          IAST
 *          
 *          
 *          
 */

    void IAST::print (const char* filename) {
        FILE* f = fopen (filename, "wr");
        assert (f);

        fprintf (f, "digraph {\n");
        dprint (f);
        fprintf (f, "}\n");

        fclose (f);
    }


/*
 *
 *
 *          Val_AST
 *          
 *          
 *          
 */

    void Val_AST::dprint (FILE* f) const {
        assert (f);

        fprintf (f, "\t%lu [label = \"val\\n%lf\"]\n", reinterpret_cast<unsigned long> (this), val_);
    }



/*
 *
 *
 *          Var_AST
 *          
 *          
 *          
 */

    Var_AST::Var_AST (const char* var) { 
        auto n = strlen (var); 
        n++;
        var_  = new char [n];  
        std::copy (var, var + n, var_);
    }


    Var_AST::Var_AST (const Var_AST& that) {
        if (that.var_) {
            auto n = strlen (that.var_); 
            n++;
            var_  = new char [n];  
            std::copy (that.var_, that.var_ + n, var_);
        }
    }


    void Var_AST::dprint (FILE* f) const {
        assert (f);

        fprintf (f, "\t%lu [label = \"var\\n%s\"]\n", reinterpret_cast<unsigned long> (this), var_);
    }


/*
 *
 *
 *          Op_AST
 *          
 *          
 *          
 */

    void Op_AST::dprint (FILE* f) const {
        assert (f);

        fprintf (f, "\t%lu [label = \"op\\n%d\"]\n", reinterpret_cast<unsigned long> (this), op_);

        if (left_) {
            fprintf (f, "\t%lu->%lu\n", reinterpret_cast<unsigned long> (this)
                                      , reinterpret_cast<unsigned long> (left_));
            left_->dprint (f);
        }
        if (right_) {
            fprintf (f, "\t%lu->%lu\n", reinterpret_cast<unsigned long> (this)
                                      , reinterpret_cast<unsigned long> (right_));
            right_->dprint (f);
        }
    }


}