#include <cstdio>
#include <cassert>
#include <sstream>
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

    void IAST::print (const char* filename) const {
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

        std::stringstream ss;
        ss << val_;
        fprintf (f, "\t%lu [label = \"val\\n%s\"]\n", reinterpret_cast<unsigned long> (this), ss.str().c_str());
    }



/*
 *
 *
 *          Var_AST
 *          
 *          
 *          
 */

    void Var_AST::dprint (FILE* f) const {
        assert (f);

        fprintf (f, "\t%lu [label = \"var\\n%s\"]\n", reinterpret_cast<unsigned long> (this), var_.c_str ());
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

        fprintf (f, "\t%lu [label = \"op\\n%s\"]\n", reinterpret_cast<unsigned long> (this), string_eq (op_));

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


/*
 *
 *
 *          Arr_AST
 *
 *
 *
 */

    void Arr_AST::dprint (FILE* f) const {
        assert (f);

        fprintf (f, "\t%lu [label = \"%s\\n", reinterpret_cast<unsigned long> (this), var_.c_str ());
        fprintf (f, "dims: %lu\\n", dims_.size());
        for (auto dim : dims_)
          fprintf(f, "%u ", dim);
        fprintf (f, "\"]\n");
    }

}
