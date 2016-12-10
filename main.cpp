#include "ast.h"
#include "lexem.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

void test_ast ();
void test_lexer ();
void test_parser ();
void test_interpreter ();


int main () {
    // test_ast ();
    // test_lexer ();
    test_parser ();
    test_interpreter ();

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

    
void test_lexer () {
    using namespace lexem;

    Lexer lr ("code.txt");
    Lexer lr2 (lr);
    Lexer lr3 (std::move (lr2));
    lr = lr3;

    ILexem* tmp = lr.cur_lexem ();
    while (1) {
        if (tmp->get_type () == VAR)
            printf ("var\t%s\n", tmp->get_var ());
        if (tmp->get_type () == VAL)
            printf ("val\t%lf\n", tmp->get_val ()); 
        if (tmp->get_type () == OP) {
            auto t = tmp->get_op ();
            printf ("op \t%s\n", op::string_eq (t));
            if (t == op::END)
                break;   
        }
        delete tmp;
        lr.next_lexem ();
        tmp = lr.cur_lexem ();
    }

    delete tmp;
}


void test_parser () {
    using namespace parser;

    Parser p ("code.txt");
    Parser p2 (p);
    Parser p7 ("code.txt");
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

    Parser p4 (Parser ("code.txt"));
    tmp = p4.build ();
    tmp->print ("test/parser4.dot");
    delete tmp;

    Parser p5 (std::move (p2));
    tmp = p5.build ();
    tmp->print ("test/parser5.dot");
    delete tmp;

    Parser p6 ("code.txt");
    p6 = p5;
    tmp = p6.build ();
    tmp->print ("test/parser6.dot");
    delete tmp;

    tmp = p7.build ();
    tmp->print ("test/parser7.dot");
    delete tmp;

    Parser p8 ("code.txt");
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


void test_interpreter () {
    parser::Parser p ("code.txt");
    ipr::Interpreter i (p.build ());
    printf ("result %lf\n", i.run ()); 
}