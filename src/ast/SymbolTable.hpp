#ifndef SYMBOLTABLE_HPP
#define SYMBOLTABLE_HPP

#include <unordered_map>
#include <vector>
#include <string>


class SymbolTable{

    public:
       
        std::unordered_map<std::string, std::vector<std::string>> symbol_table;

        /**
         * Inserts a new name inside the symbol table
         * 
         * @param var The name of the variable
         * @param type The type of the variable
         */
        void insert(const std::string& var, const std::string& type);

        /**
         * Remove a name from the symbol table
         * 
         * @param var The name of the variable
         */
        void remove(const std::string& var);

        /**
         * Determines wheter a variable is in the symbol table.
         * 
         * @param var The name of the variable
         * 
         * @returns true if the var is inside the symbol table, false else.
         */
        bool look_up(const std::string& var);

        /**
         * Returns the type of a variable
         * 
         * @param var The name of the variable
         * 
         * @returns The type of var
         */
        std::string get(const std::string& var);


};


#endif