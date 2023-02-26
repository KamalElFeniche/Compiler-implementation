LEX = flex
CC = clang++
FLAGS =  -std=c++14 `llvm-config-9 --cxxflags --ldflags --libs`
YACC = bison -d
SRCDIR = ast/
EXT = .cpp
SRCS = $(wildcard $(SRCDIR)*$(EXT))

vsopc: main.cpp lex.yy.c vsop.tab.c $(SRCS)
		$(CC) $(FLAGS) -o vsopc vsop.tab.c lex.yy.c main.cpp $(SRCS)

lex.yy.c: vsop.l
		$(LEX) vsop.l

vsop.tab.c: vsop.y
		$(YACC) vsop.y

install-tools:
	sudo apt-get install binfmt-support libclang-cpp9 libllvm9 libpipeline1 llvm-9 llvm-9-dev llvm-9-runtime llvm-9-tools
	sudo apt-get install llvm-9
	sudo mkdir -p /vsop
	sudo cp object.s /vsop