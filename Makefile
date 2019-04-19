CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -Werror -Wno-return-local-addr -std=c++17 -g3 -Wswitch-default -Wmaybe-uninitialized -Wredundant-decls ${ADD_CXXFLAGS}

all: compile_interpreter

cr: compile_interpreter run read

run:
	./interpreter

tests: compile_interpreter run_tests diff

read:
	cat output.txt

run_tests:
	echo "Running tests";							\
	number=1;									 			\
	for f in tests/input/*.txt; do 				 			\
		echo  "\nFile execution "$$f ;	\
		cp $$f input.txt; 						 			\
		./interpreter;							 			\
		echo $$f >> output.txt; 				 			\
		cp output.txt tests/output/$$number.txt; 			\
		number=`expr $$number + 1`; 			 			\
	done;

diff:
	echo "\nSearch differences";	\
	cd tests/output; for f in [1-9]*.txt; do echo $$f; diff $$f "../ideals/"$$f; done

compile_interpreter: main.o ast.o parser.o lexer.o operator.o lexem.o interpreter.o
	$(CXX) $(CXXFLAGS) main.o ast.o parser.o lexer.o operator.o lexem.o interpreter.o -o interpreter

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
	
