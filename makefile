APP = -std=c++11 -Wall ${ADD_APP}

all: compile

compile: main_cfg_test.o ast.o parser.o lexer.o ssa.o operator.o lexem.o
	$(CXX) $(APP) main_cfg_test.o ast.o parser.o ssa.o lexer.o operator.o lexem.o -o cfg

main_cfg_test.o: main_cfg_test.cpp
	$(CXX) $(APP) -c main_cfg_test.cpp

ast.o: ast.cpp
	$(CXX) $(APP) -c ast.cpp

parser.o: parser.cpp
	$(CXX) $(APP) -c parser.cpp

lexer.o: lexer.cpp
	$(CXX) $(APP) -c lexer.cpp

ssa.o: ssa.cpp
	$(CXX) $(APP) -c ssa.cpp

operator.o: operator.cpp
	$(CXX) -$(APP) -c operator.cpp

lexem.o: lexem.cpp
	$(CXX) -$(APP) -c lexem.cpp

clean:
	rm -rf *.o

