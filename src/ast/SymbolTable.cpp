#include "SymbolTable.hpp"

using namespace std;

// SymbolTable class

void SymbolTable::insert(const string& var, const string& type){
    
    if(look_up(var)){

        symbol_table.at(var).push_back(type);

    }else{

        symbol_table.insert({var, {type}});
    }
}

void SymbolTable::remove(const string& var){

    if(look_up(var)){

        if (symbol_table.at(var).size() > 1)

            symbol_table.at(var).pop_back();
    
        else

            symbol_table.erase(var);

    }
}

bool SymbolTable::look_up(const string& var){
    return symbol_table.find(var) != symbol_table.end();    
}

string SymbolTable::get(const string& var){
    return symbol_table.at(var).back();
}