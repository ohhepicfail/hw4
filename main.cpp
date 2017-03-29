#include <string>
#include <fstream>
#include <iostream>
#include "interpreter.h"


int main (int argc, char* argv[]) {
    std::string if_name = "input.txt";
    std::string of_name = "output.txt";
    if (argc > 1)
        if_name = argv[1];
    if (argc > 2)
        of_name = argv[2];
    ipr::Interpreter i (if_name);

    std::ofstream out (of_name);
    if (!out.is_open ())
        std::cout << "Can't open file " << of_name << std::endl;
    else
        out << "result " << i.run () << std::endl;

    return 0;
}