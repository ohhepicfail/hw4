#include <string>
#include <fstream>
#include <iostream>
#include "translator.h"

int main (int argc, char* argv[]) {
    std::string if_name = "input_test.txt";
    std::string of_name = "output_test.s";
    if (argc > 1)
        if_name = argv[1];
    if (argc > 2)
        of_name = argv[2];
    Translator t (if_name);
    t.run();
    t.write_file(of_name);
    return 0;
}
