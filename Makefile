CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -Werror -Wno-return-local-addr -std=c++14 -g3 -Wswitch-default -Wmaybe-uninitialized -Wredundant-decls
LCYAN = \033[1;36m
NORMAL = \033[0m
LYELLOW=\033[1;33m
MAKEFLAGS += --silent

all: compile clean

cr: compile clean run read

run:
	./interpreter

tests: compile run_tests diff

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

all_valgrind: compile clean
	valgrind ./interpreter

compile: ast.cpp lexem.cpp lexer.cpp operator.cpp parser.cpp interpreter.cpp main.cpp
	$(CXX) $(CXXFLAGS) ast.cpp lexem.cpp lexer.cpp operator.cpp parser.cpp interpreter.cpp main.cpp -o interpreter; \
	echo "${LCYAN}Compilation is completed\n";

clean:
	rm -rf *.o
	
