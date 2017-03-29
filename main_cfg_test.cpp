#include "parser.h"
#include "ssa.h"
#include <cassert>


int main (int argc, char **argv)
{
    assert (argc == 2);
    ssa_translator translator (argv[1]);
    translator.build_CFG ();
    return 0;
}
    
