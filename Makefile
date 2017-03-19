CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -Werror -Wno-return-local-addr -std=c++14 -g3 -Wswitch-default -Wmaybe-uninitialized -Wredundant-decls ${ADD_CXXFLAGS}

all: compile_cfg clean

compile_cfg: main_cfg_test.o ast.o parser.o lexer.o ssa.o operator.o lexem.o
	$(CXX) $(CXXFLAGS) main_cfg_test.o ast.o parser.o ssa.o lexer.o operator.o lexem.o -o cfg

main_cfg_test.o: main_cfg_test.cpp
	$(CXX) $(CXXFLAGS) -c main_cfg_test.cpp

ast.o: ast.cpp
	$(CXX) $(CXXFLAGS) -c ast.cpp

parser.o: parser.cpp
	$(CXX) $(CXXFLAGS) -c parser.cpp

lexer.o: lexer.cpp
	$(CXX) $(CXXFLAGS) -c lexer.cpp

ssa.o: ssa.cpp
	$(CXX) $(CXXFLAGS) -c ssa.cpp

operator.o: operator.cpp
	$(CXX) $(CXXFLAGS) -c operator.cpp

lexem.o: lexem.cpp
	$(CXX) $(CXXFLAGS) -c lexem.cpp

clean:
	rm -rf *.o
	
