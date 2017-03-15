#include "ast.h"
#include "lexem.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

void test_ast ();
void test_lexer (int argc, char* argv[]);
void test_parser (int argc, char* argv[]);
void test_interpreter (int argc, char* argv[]);


int main (int argc, char* argv[]) {
#if 0
    test_ast ();
    test_lexer ();
    test_parser ();
#endif
    test_interpreter (argc, argv);

    return 0;
}


void test_ast () {
    using namespace ast;
     
    IAST* ast = new Val_AST (.4);
    IAST* ast2 = new Var_AST ("hello");
    IAST* ast3 = new Op_AST (op::ADD, ast, ast2);
    IAST* ast4 = new Var_AST ("bum");
    IAST* ast5 = new Op_AST (op::ASSIGN, ast4, ast3);
    IAST* nast = ast5->clone ();
    ast5->print ("test/ast1.dot");
    nast->print ("test/ast2.dot");
    delete ast5; 
    delete nast;  
}

    
void test_lexer (int argc, char* argv[]) {
    using namespace lexem;

    const char* if_name = "input.txt";
    if (argc > 1)
        if_name = argv[1];

    Lexer lr (if_name);
    Lexer lr2 (lr);
    Lexer lr3 (std::move (lr2));
    lr = lr3;
    lr = lr;

    Lexem tmp = lr.get_cur_lexem ();
    while (1) {
        if (tmp.get_type () == VAR)
            printf ("var\t%s\n", tmp.get_var ().c_str ());
        if (tmp.get_type () == VAL)
            printf ("val\t%lf\n", tmp.get_val ()); 
        if (tmp.get_type () == OP) {
            auto t = tmp.get_op ();
            printf ("op \t%s\n", op::string_eq (t));
            if (t == op::END)
                break;   
        }
        lr.next_lexem ();
        tmp = lr.get_cur_lexem ();
    }
}


void test_parser (int argc, char* argv[]) {
    using namespace parser;

    const char* if_name = "input.txt";
    if (argc > 1)
        if_name = argv[1];

    Parser p (if_name);
    Parser p2 (p);
    Parser p7 (if_name);
    p7 = p;
    IAST* tmp = p.build ();
    tmp->print ("test/parser1.dot");
    delete tmp;
    tmp = p.build ();
    delete tmp;

    tmp = p2.build ();
    tmp->print ("test/parser2.dot");
    delete tmp;

    Parser p3 (p2);
    tmp = p3.build ();
    tmp->print ("test/parser3.dot");
    delete tmp;

    Parser ptmp (if_name);
    Parser p4 (std::move (ptmp));
    tmp = p4.build ();
    tmp->print ("test/parser4.dot");
    delete tmp;

    Parser p5 (std::move (p2));
    tmp = p5.build ();
    tmp->print ("test/parser5.dot");
    delete tmp;

    Parser p6 (if_name);
    p6 = p5;
    p6 = p6;
    tmp = p6.build ();
    tmp->print ("test/parser6.dot");
    delete tmp;

    tmp = p7.build ();
    tmp->print ("test/parser7.dot");
    delete tmp;

    Parser p8 (if_name);
    Parser p9 (p8);
    delete p8.build ();
    Parser p10 (p8);
    p8 = std::move (p9);
    tmp = p8.build ();
    tmp->print ("test/parser8.dot");
    delete tmp;
    p8 = std::move (p10);
    tmp = p8.build ();
    tmp->print ("test/parser9.dot");
    delete tmp;

}


void test_interpreter (int argc, char* argv[]) {
    using namespace parser;
    using namespace ipr;

    const char* if_name = "input.txt";
    const char* of_name = "output.txt";
    if (argc > 1)
        if_name = argv[1];
    if (argc > 2)
        of_name = argv[2];
    Parser p (if_name);
    Interpreter i (p.build ());
    Interpreter i1 (i);
    FILE* of = fopen (of_name, "wb");
    assert (of); 
    fprintf (of, "result %lf\n", i.run ());
    fclose (of);

    Interpreter i2 (i); 
    Interpreter i3 (std::move (i1));
    Interpreter i4 (std::move (i2));

    Interpreter i5 (i3);
    Interpreter i6 (i3);
    i5 = i4;
    i6 = std::move (i4);

    i5 = i5;
}