#ifndef CODEGENERATOR_HPP
#define CODEGENERATOR_HPP

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "llvm/IR/Type.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

/**
 * Determines if a type is int32
 * 
 * @param type The llvm Type
 * 
 * @returns true if the type is int32, false else
 */
bool is_int32(llvm::Type* type);

/**
 * Determines if a type is string
 * 
 * @param type The llvm Type
 * 
 * @returns true if the type is string, false else
 */
bool is_string(llvm::Type* type);

/**
 * Determines if a type is bool
 * 
 * @param type The llvm Type
 * 
 * @returns true if the type is bool, false else
 */
bool is_bool(llvm::Type* type);

/**
 * Determines if a type is a class
 * 
 * @param type The llvm Type
 * 
 * @returns true if the type is a class, false else
 */
bool is__class(llvm::Type* type);

/**
 * Determines if a type is unit
 * 
 * @param type The llvm Type
 * 
 * @returns true if the type is unit, false else
 */
bool is_unit(llvm::Type* type);

/**
 * Converts a llvm Type to string
 * 
 * @param type The llvm Type
 * 
 * @returns A string representation of the type
 */
std::string type_to_string(llvm::Type* type);

/**
 * Checks if two types are the same
 * 
 * @param type_1 The first type
 * @param type_2 The second type
 * 
 * @returns true if the types are the same
 */
bool is_same_as(llvm::Type* type_1, llvm::Type* type_2);

class CodeGenerator {

    public: 

            std::shared_ptr<llvm::LLVMContext> context;
            std::shared_ptr<llvm::IRBuilder<>> builder;
            std::shared_ptr<llvm::Module> module;
            std::unordered_map<std::string, std::vector<llvm::Value*>> scope;

            /**
             * Create a new CodeGenerator object
             * 
             * @param name The name of the CodeGenerator
             * 
             * @returns a new CodeGenerator Object.
             */
            CodeGenerator(const std::string& name);

            /**
             * Inserts a new name inside the symbol table,
             * but does not allocate memory.
             * 
             * @param var The name of the variable
             * @param value The llvm value of the variable
             */
            void insert(const std::string& var, llvm::Value* val);

            /**
             * Remove a name inside the symbol table,
             * 
             * @param var The name of the variable
             * @param value The llvm value of the variable
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
             * Returns the value of a variable
             * 
             * @param var The name of the variable
             * 
             * @returns The value of var
             */
            llvm::Value* get_val(const std::string& var);

            /**
             * Returns the type of a variable
             * 
             * @param var The name of the variable
             * 
             * @returns The type of var
             */
            llvm::Type* get_type(const std::string& var);

            /**
             * Inserts a new name inside the symbol table and
             * allocate memory for it
             * 
             * @param var The name of the variable
             * @param type The llvm type of the variable
             */
            void allocate(const std::string& var, llvm::Type* type);

            /**
             * Stores a variable in memory
             * 
             * @param var The name of the variable
             * @param val The llvm value of the variable
             */
            void store(const std::string& var, llvm::Value* val);

            /**
             * Load a variable from memory
             * 
             * @param var The name of the variable
             * 
             * @returns The llvm value of the variable
             */
            llvm::Value* load(const std::string &var);

            /**
             * Converts a string to a llvm type
             * 
             * @param type The string representing the type
             * 
             * @returns a llvm Type corresponding to the type
             */
            llvm::Type* to_type(const std::string& type);

            /**
             * Get the default value of a llvm Type
             * 
             * @param type The llvm Type
             * 
             * @returns The default value of the llvm Type
             */
            llvm::Value* default_val(llvm::Type* type);

            /**
             * Get the default value of a string type
             * 
             * @param type The type (in string)
             * 
             * @returns The default value of the type
             */
            llvm::Value* default_val(const std::string& type);

            /**
             * Get the string representation of the llvm code
             * contained inside the module
             * 
             * @returns The code inside the module
             */
            std::string print();

            /**
             * Performs the following optimization passes:
             * "peephole" optimizations, reassociate expressions,
             * eliminate common subexpressions, simplify the control
             * flow graph (delete unreachable blocks, ...).
             */
            void optimizer();
};

#endif