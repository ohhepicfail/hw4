CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -Werror -Wno-return-local-addr -std=c++14 -g3 -Wswitch-default -Wmaybe-uninitialized	-Wredundant-decls

all: compile clean
	./interpreter

all_valgrind: compile clean
	valgrind ./interpreter

compile: ast.cpp lexem.cpp lexer.cpp operator.cpp parser.cpp interpreter.cpp main.cpp
	$(CXX) $(CXXFLAGS) ast.cpp lexem.cpp lexer.cpp operator.cpp parser.cpp interpreter.cpp main.cpp -o interpreter

clean:
	rm -rf *.o
	
