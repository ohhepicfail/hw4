CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -Werror -Wno-return-local-addr -std=c++14 -g3 -Wswitch-default -Wmaybe-uninitialized -Wredundant-decls ${ADD_CXXFLAGS}
LCYAN = \033[1;36m
NORMAL = \033[0m
LYELLOW=\033[1;33m
MAKEFLAGS += --silent

all: compile_interpreter clean

cr: compile_interpreter clean run read

run:
	./interpreter

tests: compile_interpreter run_tests diff

read:
	cat output.txt

run_tests:
	echo "${LCYAN}Running tests";							\
	number=1;									 			\
	for f in tests/input/*.txt; do 				 			\
		echo  "${LYELLOW}\nFile execution "$$f "${NORMAL}";	\
		cp $$f input.txt; 						 			\
		./interpreter;							 			\
		echo $$f >> output.txt; 				 			\
		cp output.txt tests/output/$$number.txt; 			\
		number=`expr $$number + 1`; 			 			\
	done;

diff:
	echo "${LCYAN}\nSearch differences$(NORMAL)";	\
	cd tests/output; for f in [1-9]*.txt; do echo $$f; diff $$f "../ideals/"$$f; done

compile_interpreter: main.o ast.o parser.o lexer.o operator.o lexem.o interpreter.o
	$(CXX) $(CXXFLAGS) main.o ast.o parser.o lexer.o operator.o lexem.o interpreter.o -o interpreter; \
	echo "${LCYAN}Compilation is completed\n${NORMAL}";

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

interpreter.o: interpreter.cpp
	$(CXX) $(CXXFLAGS) -c interpreter.cpp	

ast.o: ast.cpp
	$(CXX) $(CXXFLAGS) -c ast.cpp

parser.o: parser.cpp
	$(CXX) $(CXXFLAGS) -c parser.cpp

lexer.o: lexer.cpp
	$(CXX) $(CXXFLAGS) -c lexer.cpp

operator.o: operator.cpp
	$(CXX) $(CXXFLAGS) -c operator.cpp

lexem.o: lexem.cpp
	$(CXX) $(CXXFLAGS) -c lexem.cpp

clean:
	rm -rf *.o
	
