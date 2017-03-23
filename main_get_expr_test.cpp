#include "parser.h"
#include <iostream>

std::ostream& operator<< (std::ostream& stream, std::stack<const ast::IAST*>& stack)
{
    if (stack.empty ())
    {
       stream << "empty" << std::endl;
       return stream;
    }
    while (!stack.empty ())
    {
        const ast::IAST* node = stack.top ();
        stack.pop ();
        auto general_type = node->get_type();
        switch (general_type)
        {
            case Type::VAR: stream << "VAR " << node->get_var () << std::endl;
                            break;
            case Type::VAL: stream << "VAL " << node->get_val () << std::endl;
                            break;
            case Type::OP:  stream << string_eq (node->get_op ()) << std::endl;
                            break;
            default:    assert (!"NOOOOO!");
        }
    }
    return stream;
}

int main (int argc, char **argv)
{
    assert (argc == 2);
    parser::Parser p(argv[1], parser::INTERPRETER);
    p.get_next();
    p.get_next();
    std::cout << p.get_next_expr() << std::endl;
    return 0;
}
