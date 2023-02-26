#include <iostream>
#include <fstream>
#include "vsop.tab.h"
#include "ast/ast.hpp"
#include "ast/CodeGenerator.hpp"

extern int yyparse(void);
std::string file_name;
extern FILE* yyin;
int mode = 0;
VSOPProgram* vsop;
VSOPList<Class> program;

int main(int argc, char const *argv[])
{   
    if (argc != 3 && argc != 2){
        std::cerr << "vsopc: bad number of arguments" << std::endl;
        return 1;
    }
    std::string option = "";
    if (argc == 3)
        option = argv[1];

    if (option == "-lex" || option == "-l")
        mode = START_LEX;

    if (option == "-p" || option == "-c" || option == "-i" || option == "")
        mode = START_PARSE;

    if (mode == 0){
        std::cerr << "vsopc: error in arguments" << std::endl;
        return 1;
    }

    FILE* file;
    if (argc == 3)
        file = fopen(argv[2], "r");
    else
        file = fopen(argv[1], "r");

    if (!file){
        std::cerr << "vsopc: no such file or directory" << std::endl;
        return 1;
    }

    yyin = file;
    if (argc == 3)
        file_name = argv[2];
    else
        file_name = argv[1];
    yyparse();
    if (option == "-p" || option == "-c" || option == "-i" || option == ""){
        vsop = new VSOPProgram(program);
        vsop->file_name = file_name;
        
        if (option == "-p")
            std::cout << vsop->print() << std::endl;
        else {
            SymbolTable scope;
            vsop->declaration();
            vsop->semanticAnalysis(*vsop, scope);
            if (vsop->nb_errors != 0)
                return vsop->nb_errors;
            if (option == "-c"){
                std::cout << vsop->print() << std::endl;
                return 0;
            }
            
            CodeGenerator coder("test");
            vsop->pre_codegen(*vsop, coder);
            vsop->codegen(*vsop, coder);

            if (option == "-i"){
                std::cout << coder.print();
                return 0;
            }

            coder.optimizer();

            std::string basename = file_name.substr(0, file_name.find_last_of('.'));
            
            std::ofstream out(basename + ".ll");
            out << coder.print();
            out.close();

            std::string cmd = "llc-9 " + basename + ".ll -O2";
            system(cmd.c_str());

            cmd = "clang " + basename + ".s /vsop/object.s -lm -o " + basename;
            system(cmd.c_str());

        }

        delete vsop;

    }

    return 0;
}
