#include "ast.h"
#include "lexem.h"
#include "lexer.h"
#include "parser.h"

void test_ast ();
void test_lexer ();
void test_parser ();


int main () {
    // test_ast ();
    // test_lexer ();
    test_parser ();

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
    ast5->print ("hello.dot");
    nast->print ("nast.dot");
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
    while (1) {
        IAST* tmp = p.build ();
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
        tmp->print ("parser.dot");
        delete tmp;
    }
}