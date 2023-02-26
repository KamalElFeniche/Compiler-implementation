/**
 * This file defines all the classes related to the abstract syntax tree
 * 
 */

#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include "SymbolTable.hpp"
#include "CodeGenerator.hpp"
#include "utils.hpp"


class VSOPProgram;  // Class declaration here, definition below


/**
 * This class represents a node in the AST
 * 
 */
class Node
{

    public:

        Node() {}   // Constructor
        virtual ~Node() {}  // Destructor

        int line = 1;   // line in the file
        int col = 1;    // Column in the file
        std::string file_name;


        /**
         * Computes the string representation of the node.
         * 
         * @returns the string representation of the node.
         */
        virtual std::string print() = 0;

        /**
         * Function that enter in the scope of the node.
         * 
         * @param scope SymbolTable whose scope will be modified.
         */
        virtual void enter_scope(SymbolTable& scope) {}

        /**
         * Function that exits the scope of the node
         * 
         * @param scope SymbolTable whose scope will be modified.
         */
        virtual void exit_scope(SymbolTable& scope) {}

        /**
         * Prints a semantic error on the standard error output
         * 
         * @param msg The message to print
         */
        void semanticError(const std::string& msg);

        /**
         * Generate code for the node
         * 
         * @param prog VSOPProgam which contains the node
         * @param coder CodeGenerator which will generate the code
         */
        virtual void codegen(VSOPProgram& prog, CodeGenerator& coder) {}
};


class Class;    // Class declaration here, definition below
class Block;    // Class declaration here, definition below
class Field;    // Class declaration here, definition below


/**
 * This class represents a list of nodes
 * 
 * @tparam T the type to be stored in the list
 */
template <typename T>
class VSOPList : public Node{
    public:
            std::vector<std::shared_ptr<T>> list;

            explicit VSOPList() {}; // Constructor

            /**
             * Creates a new VSOPList
             * 
             * @param elements A vector of elements to be stored in the list
             * 
             * @returns A new VSOPList which has a list of elements
             */
            explicit VSOPList(std::vector<std::shared_ptr<T>> elements){
                for(std::shared_ptr<T> element : elements)
                    list.push_back(element);
            }

            /**
             * Creates a new VSOPList
             * 
             * @param elements A vector of elements to be stored in the list
             * 
             * @returns A new VSOPList which has a list of elements
             */
            explicit VSOPList(std::vector<T*> elements){
                for(T* element : elements)
                    list.push_back(std::shared_ptr<T>(element));
            }

            /**
             * Push a new node inside the list
             * 
             * @param element The element to be pushed in the list
             */
            void push(std::shared_ptr<T> element){
                list.push_back(element);
            }

            /**
             * Push a new node inside the list
             * 
             * @param element The element to be pushed in the list
             */
            void push(T* element){
                list.push_back(std::shared_ptr<T>(element));
            }

            /**
             * Reverse the VSOPList
             * 
             * @returns The reversed VSOPList
             */
            VSOPList& reverse(){
                std::reverse(list.begin(), list.end());
                return *this;
            }

            /**
             * Computes the string representation of the VSOPList
             * 
             * @returns the string representation of the VSOPList.
             */
            virtual std::string print(){
                if (list.empty())
                    return "[]";
                
                std::string text = "[" + list.front()->print();
                auto element = list.begin() + 1;
                while(element != list.end()){
                    text += "," + (*element)->print();
                    element++;
                }

                return text + "]";
            }

            /**
             * Function that enter in the scope of the VSOPList
             * 
             * @param scope SymbolTable whose scope will be modified.
             */
            void enter_scope(SymbolTable& scope){

                for (auto& element : list)
                    element->enter_scope(scope);
          
            }

            /**
             * Function that exits the scope of the VSOPList.
             * 
             * @param scope SymbolTable whose scope will be modified.
             */
            void exit_scope(SymbolTable& scope){

                for (auto& element : list)
                    element->exit_scope(scope);
            }

            /**
             * Performs the semantic analysis of the VSOPList
             * 
             * @param prog The VSOPProgram which contains the VSOPList
             * @param scope The SymbolTable which contains the scope
             */
            void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){
                
                for(auto& element : list)
                    element->semanticAnalysis(prog, scope);
            }

            /**
             * Generate code for the VSOPList.
             * 
             * @param prog VSOPProgam which contains the VSOPList
             * @param coder CodeGenerator which will generate the code
             */
            virtual void codegen(VSOPProgram& prog, CodeGenerator& coder){

                for (auto& element : list)
                    element->codegen(prog, coder);
            }
};

/**
 * This class represents a VSOPProgram (aka an AST)
 */
class VSOPProgram : public Node{
    public:
            VSOPList<Class> program;
            std::unordered_map<std::string, std::shared_ptr<Class>> class_table;
            int nb_errors = 0;

            explicit VSOPProgram(); // Constructor

            /**
             * Creates a new object VSOPProgram
             *
             * @param program The list of classes
             * 
             * @returns A new VSOPProgram with the list of classes.
             */
            explicit VSOPProgram(const VSOPList<Class>& program);

            /**
             * Computes the string representation of the VSOPProgram
             * 
             * @returns The string representation of the VSOPProgram
             */
            virtual std::string print();

            /**
             * Declares all the classes that are present in the list
             * of classes, check if any class is redefined,
             * checks for cycles in inheritance, and check if each class
             * does not override something inherited.
             * 
             */
            void declaration();

            /**
             * Performs the semantic analysis on the VSOPProgram.
             * 
             * @param prog The VSOPProgram (itself)
             * @param scope The SymbolTable which represents the scope
             */
            void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * Declares all the classes inside the CodeGenerator
             * 
             * @param prog The VSOPProgram (itself)
             * @param coder The CodeGenerator in which everything will be declared
             */
            void pre_codegen(VSOPProgram& prog, CodeGenerator& coder);

            /**
             * Generate the code for the VSOPProgram.
             * 
             * @param prog The VSOPProgram (itself)
             * @param coder The CodeGenerator which will generate the code.
             */
            virtual void codegen(VSOPProgram& prog, CodeGenerator& coder);
};

/**
 * This class represents a Formal
 */
class Formal : public Node{
    public:
            std::string name;
            std::string type;

            explicit Formal();  // Constructor

            /**
             * Creates a new Formal
             * 
             * @param name The name of the Formal
             * @param type The type of the Formal
             *
             * @returns a new Formal Object
             */
            explicit Formal(const std::string& name, const std::string& type);

            /**
             * Computes the string representation of the Formal
             * 
             * @returns The string representation of the Formal
             */
            virtual std::string print();

            /**
             * Enters the scope of the Formal
             * 
             * @param scope The SymbolTable which contains the scope
             */
            void enter_scope(SymbolTable& scope);

            /**
             * Exits the scope of the Formal
             * 
             * @param scope The SymbolTable which contains the scope.
             */
            void exit_scope(SymbolTable& scope);

            /**
             * Performs the semantic Analysis of the Formal
             * 
             * @param prog The VSOPProgram which contains the Formal
             * @param scope The SymbolTable which represents the scope
             */
            void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * Get the type of the Formal
             * 
             * @param prog The VSOPProgram which contains the Formal
             * @param scope The SymbolTable which represents the scope
             */
            std::string getType(VSOPProgram& prog, SymbolTable& scope);
};

/**
 * This class represents a Method.
 */
class Method : public Node{
    public:
            std::string name;
            std::string return_type;

            VSOPList<Formal> formal;
            std::shared_ptr<Block> block;
            std::unordered_map<std::string, std::shared_ptr<Formal>> formal_table;

            Class* parent = nullptr;    // Class which implements this method.
            int index_vtable;

            explicit Method();  // Constructor

            /**
             * Creates a new Method object
             * 
             * @param name The name of the Method
             * @param return_type The return type of the Method
             * @param formal The list of formals of the Method
             * @param block The block of the Method
             * 
             * @returns a new Method object.
             */
            explicit Method(const std::string& name, const std::string& return_type, const VSOPList<Formal>& formal, std::shared_ptr<Block> block);

            /**
             * Creates a new Method object
             * 
             * @param name The name of the Method
             * @param return_type The return type of the Method
             * @param formal The list of formals of the Method
             * @param block The block of the Method
             * 
             * @returns a new Method object.
             */
            explicit Method(const std::string& name, const std::string& return_type, const VSOPList<Formal>& formal, Block* block);

            /**
             * Computes the string representation of the Method
             * 
             * @returns the string representation of the Method
             */
            virtual std::string print();

            /**
             * Check if the declaration of the Method is correct,
             * i.e. it checks if the formal are not redefined.
             * 
             * @param prog The VSOPProgram which contains this Method.
             */
            void declaration(VSOPProgram& prog);

            /**
             * Enters the scope of the Method
             * 
             * @param scope The symbolTable which represents the scope
             */
            void enter_scope(SymbolTable& scope);

            /**
             * Exits the scope of the Method
             * 
             * @param scope The symbolTable which represents the scope
             */
            void exit_scope(SymbolTable& scope);

            /**
             * Performs the semantic Analysis of the Method
             * 
             * @param prog The VSOPProgram which contains the Method
             * @param scope The SymbolTable which represents the scope
             */
            void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * Declares the Method inside the CodeGenerator
             * 
             * @param prog The VSOPProgram which contains the Method
             * @param coder The CodeGenerator in which everything will be declared
             */
            void pre_codegen(VSOPProgram& prog, CodeGenerator& coder);

            /**
             * Generate the code for the Method.
             * 
             * @param prog The VSOPProgram which contains the Method
             * @param coder The CodeGenerator which will generate the code.
             */
            virtual void codegen(VSOPProgram& prog, CodeGenerator& coder);

            /**
             * Get the name of the function inside the CodeGenerator
             * 
             * @return the name of the function inside the CodeGenerator
             */
            std::string get_name();

            /**
             * Get the function inside the CodeGenerator
             * 
             * @return A pointer towards the function inside the CodeGenerator
             */
            llvm::Function* get_function(CodeGenerator& coder);

            /**
             * Get the type of the function inside the CodeGenerator
             * 
             * @return A pointer towards the type of the function inside
             * the CodeGenerator
             */
            llvm::FunctionType* get_type(CodeGenerator& coder);
};

/**
 * This class represents a Class (!).
 */
class Class : public Node{
    public:
            std::string name;
            std::string parent;

            std::shared_ptr<Class> parent_class = nullptr;

            VSOPList<Field> field;
            VSOPList<Method> method;
            std::unordered_map<std::string, std::shared_ptr<Field>> field_table;
            std::unordered_map<std::string, std::shared_ptr<Method>> method_table;

            explicit Class();   // Constructor

            /**
             * Creates a new Class object
             * 
             * @param name The name of the Class
             * @param parent The parent's name of the Class
             * @param field The list of field of the Class
             * @param method The list of Method of the Class
             * 
             * @returns a new Class object.
             */
            explicit Class(const std::string& name, const std::string& parent, const VSOPList<Field>& field, const VSOPList<Method>& method);

            /**
             * Computes the string representation of the Class
             * 
             * @returns the string representation of the Class
             */
            virtual std::string print();

            /**
             * Check if the declaration of the Class is correct,
             * i.e. it checks if the fields and method are not redefined.
             * 
             * @param prog The VSOPProgram which contains this Class.
             */
            void declaration(VSOPProgram& prog);

            /**
             * Checks if the Class does not overrides an inherited field 
             * or method.
             * 
             * @param prog The VSOPProgram which contains this Class
             */
            void override(VSOPProgram& prog);

            /**
             * Enters the scope of the Class
             * 
             * @param scope The symbolTable which represents the scope
             */
            void enter_scope(SymbolTable& scope);

            /**
             * Exits the scope of the Class
             * 
             * @param scope The symbolTable which represents the scope
             */
            void exit_scope(SymbolTable& scope);

            /**
             * Performs the semantic Analysis of the Class
             * 
             * @param prog The VSOPProgram which contains the Class
             * @param scope The SymbolTable which represents the scope
             */
            void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * Get the type of the Class
             * 
             * @param prog The VSOPProgram which contains the Class
             * @param scope The SymbolTable which represents the scope
             * 
             * @returns the type of the Class
             */
            std::string getType(VSOPProgram& prog, SymbolTable& scope);

            /**
             * Declares the Class inside the CodeGenerator
             * 
             * @param prog The VSOPProgram which contains the Class
             * @param coder The CodeGenerator in which everything will be declared
             */
            void pre_codegen(VSOPProgram& prog, CodeGenerator& coder);

            /**
             * Generate the code for the Class.
             * 
             * @param prog The VSOPProgram which contains the Class
             * @param coder The CodeGenerator which will generate the code.
             */
            virtual void codegen(VSOPProgram& prog, CodeGenerator& coder);

            /**
             * Get the name of the Class inside a CodeGenerator
             * 
             * @returns the name of the Class inside a CodeGenerator
             */
            std::string struct_name();

            /**
             * Determines if the Class is declared inside a CodeGenerator
             * 
             * @param coder The CodeGenerator
             * 
             * @returns true if it is declared, false else.
             */
            bool is_declared(CodeGenerator& coder);

            /**
             * Get the type of the Class inside the CodeGenerator
             * 
             * @param coder the CodeGenerator
             * 
             * @returns A pointer towards the class structure inside the CodeGenerator
             */
            llvm::StructType* get_type(CodeGenerator& coder);

};

/**
 * This class represents a Field.
 * It is an abstract class.
 */
class Expr: public Node
{
    public:

            std::string _type = "";
            llvm::Value* expr_value = nullptr;

            /**
             * Get the type of the Expr
             * 
             * @param prog The VSOPProgram which contains the Expr
             * @param scope The SymbolTable which represents the scope.
             * 
             * @returns The type of the Expr
             */
            virtual std::string getType(VSOPProgram& prog, SymbolTable& scope) = 0;

            /**
             * Performs the semantic Analysis of the Expr
             * 
             * @param prog The VSOPProgram which contains the Expr
             * @param scope The SymbolTable which represents the scope
             */
            virtual void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope) = 0;

            /**
             * Generate the code for the Expr.
             * 
             * @param prog The VSOPProgram which contains the Expr
             * @param coder The CodeGenerator which will generate the code.
             */
            virtual void codegen(VSOPProgram& prog, CodeGenerator& coder);

            /**
             * Generate the code for the Expr
             * 
             * @param prog The VSOPProgram which contains the Expr
             * @param coder The CodeGenerator which will generate the code.
             * 
             * @returns the llvm value which represents the SSA value of the Expr.
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder) = 0;

            /**
             * Get the llvm type of the Expr
             * 
             * @returns the llvm type of the Expr
             */
            llvm::Type* get_llvm_type();

            /**
             * Get the llvm value of the Expr
             * 
             * @returns the llvm value of the Expr.
             */
            llvm::Value* get_value();

};


class Field : public Expr{
    public:
            std::string name;
            std::string type;
            std::shared_ptr<Expr> init;
            int index_vtable;

            explicit Field();   // Constructor

            /**
             * Creates a new Field object
             * 
             * @param name The name of the Field
             * @param type The type of the Field
             * @param init The init expression of the Field (can be nullptr)
             * 
             * @returns a new Field object.
             */
            explicit Field(const std::string& name, const std::string& type, std::shared_ptr<Expr> init);

            /**
             * Creates a new Field object
             * 
             * @param name The name of the Field
             * @param type The type of the Field
             * @param init The init expression of the Field (can be nullptr)
             * 
             * @returns a new Field object.
             */
            explicit Field(const std::string& name, const std::string& type, Expr* init);

            /**
             * @see Expr
             */
            virtual std::string print();

            /**
             * @see Expr
             */
            virtual std::string getType(VSOPProgram& prog, SymbolTable& scope);

            /**
             * Enters the scope of the Field
             * 
             * @param scope The SymbolTable which represents the scope
             */
            void enter_scope(SymbolTable& scope);

            /**
             * Exits the scope of the Field
             * 
             * @param scope The SymbolTable which represents the scope
             */
            void exit_scope(SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder);
};

class Block: public Expr{
    public:
            VSOPList<Expr> expr;

            explicit Block();   // Constructor

            /**
             * Creates a new Block object
             * 
             * @param expr The list of Expr of the Block
             * 
             * @returns a new Block object.
             */
            explicit Block(const VSOPList<Expr>& expr);

            /**
             * @see Expr
             */
            virtual std::string print();

            /**
             * @see Expr
             */
            virtual std::string getType(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder);
};


class Let : public Expr{
    public:
            std::string name;
            std::string type;
            std::shared_ptr<Expr> init;
            std::shared_ptr<Expr> scope;

            explicit Let(); // Constructor

            /**
             * Creates a new Let object
             * 
             * @param name The name of the Let
             * @param type The type of the Let
             * @param init The init expression of the Let (can be nullptr)
             * @param scope The scope of the Let
             * 
             * @returns a new Let object.
             */
            explicit Let(const std::string& name, const std::string& type, std::shared_ptr<Expr> init, std::shared_ptr<Expr> scope);

            /**
             * Creates a new Let object
             * 
             * @param name The name of the Let
             * @param type The type of the Let
             * @param init The init expression of the Let (can be nullptr)
             * @param scope The scope of the Let
             * 
             * @returns a new Let object.
             */
            explicit Let(const std::string& name, const std::string& type, Expr* init, Expr* scope);
            
            /**
             * @see Expr
             */
            virtual std::string print();

            /**
             * @see Expr
             */
            virtual std::string getType(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * Enters the scope of the Let
             * 
             * @param scope The Symboltable which represents the scope
             */
            void enter_scope(SymbolTable& scope);

            /**
             * Exits the scope of the Let
             * 
             * @param scope The Symboltable which represents the scope
             */
            void exit_scope(SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder);
};

class Call : public Expr{
    public:
            std::shared_ptr<Expr> obj;
            std::string name;
            VSOPList<Expr> arguments;

            explicit Call();    // Constructor

            /**
             * Creates a new Call object
             * 
             * @param obj The object of the Call
             * @param name The name of the Call
             * @param arguments The arguments of the Call
             * 
             * @returns a new Call object.
             */
            explicit Call(std::shared_ptr<Expr> obj, const std::string& name, const VSOPList<Expr>& arguments);

            /**
             * Creates a new Call object
             * 
             * @param obj The object of the Call
             * @param name The name of the Call
             * @param arguments The arguments of the Call
             * 
             * @returns a new Call object.
             */
            explicit Call(Expr* obj, const std::string& name, const VSOPList<Expr>& arguments);

            /**
             * @see Expr
             */
            virtual std::string print();

            /**
             * @see Expr
             */
            virtual std::string getType(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder);
};

class If : public Expr{
    public:
            std::shared_ptr<Expr> cond;
            std::shared_ptr<Expr> then;
            std::shared_ptr<Expr> else_expr;

            explicit If();  // Constructor

            /**
             * Creates a new If object
             * 
             * @param cond The condition of the If
             * @param then The then of the If
             * @param else_expr The else of the If
             * 
             * @returns a new If object.
             */
            explicit If(std::shared_ptr<Expr> cond, std::shared_ptr<Expr> then, std::shared_ptr<Expr> else_expr);
            
            /**
             * Creates a new If object
             * 
             * @param cond The condition of the If
             * @param then The then of the If
             * @param else_expr The else of the If
             * 
             * @returns a new If object.
             */
            explicit If(Expr* cond, Expr* then, Expr* else_expr);
            
            /**
             * @see Expr
             */
            virtual std::string print();

            /**
             * @see Expr
             */
            virtual std::string getType(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder);
};



class While : public Expr{
    public:
            std::shared_ptr<Expr> cond;
            std::shared_ptr<Expr> body;

            explicit While();   // Constructor

            /**
             * Creates a new While object
             * 
             * @param cond The condition of the While
             * @param body The body of the While
             * 
             * @returns a new While object.
             */
            explicit While(std::shared_ptr<Expr> cond, std::shared_ptr<Expr> body);

            /**
             * Creates a new While object
             * 
             * @param cond The condition of the While
             * @param body The body of the While
             * 
             * @returns a new While object.
             */
            explicit While(Expr* cond, Expr* body);
            
            /**
             * @see Expr
             */
            virtual std::string print();

            /**
             * @see Expr
             */
            virtual std::string getType(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder);
};


class New : public Expr{
    public:
            std::string type;

            explicit New(); // Constructor

            /**
             * Creates a new New object
             * 
             * @param type The type of the New
             * 
             * @returns a new New object.
             */
            explicit New(const std::string& type);

            /**
             * @see Expr
             */
            virtual std::string print();

            /**
             * @see Expr
             */
            virtual std::string getType(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder);
};

class Boolean : public Expr{
    public:
            bool boolean;

            explicit Boolean(); // Constructor

            /**
             * Creates a new Boolean object
             * 
             * @param boolean The boolean of the Boolean
             * 
             * @returns a new Boolean object.
             */
            explicit Boolean(bool boolean);

            /**
             * @see Expr
             */
            virtual std::string print();

            /**
             * @see Expr
             */
            virtual std::string getType(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder);
};

class BinOp : public Expr{
    public:
            enum Value {EQUAL, LOWER, LOWER_EQ, PLUS, MINUS, TIMES, DIV, POW, AND};
            Value value;
            std::shared_ptr<Expr> right;
            std::shared_ptr<Expr> left;

            explicit BinOp();   // Constructor

            /**
             * Creates a new BinOp object
             * 
             * @param value The type of operation of the BinOp
             * @param left The left hand side of the BinOp
             * @param right The right hand side of the BinOp
             * 
             * @returns a new BinOp object.
             */
            explicit BinOp(Value value, std::shared_ptr<Expr> left, std::shared_ptr<Expr> right);

            /**
             * Creates a new BinOp object
             * 
             * @param left The left hand side of the BinOp
             * @param right The right hand side of the BinOp
             * 
             * @returns a new BinOp object.
             */
            explicit BinOp(Value value, Expr* left, Expr* right);

            /**
             * @see Expr
             */
            virtual std::string print();

            /**
             * @see Expr
             */
            virtual std::string getType(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder);
};


class Assign : public Expr{
    public: 
            std::string name;
            std::shared_ptr<Expr> expr;

            explicit Assign();  // COnstructor

            /**
             * Creates a new Assign object
             * 
             * @param name The name of the Assign
             * @param expr The right hand side of the Assign
             * 
             * @returns a new Assign object.
             */
            explicit Assign(const std::string& name, std::shared_ptr<Expr> expr);

            /**
             * Creates a new Assign object
             * 
             * @param name The name of the Assign
             * @param expr The right hand side of the Assign
             * 
             * @returns a new Assign object.
             */
            explicit Assign(const std::string& name, Expr* expr);

            /**
             * @see Expr
             */
            virtual std::string print();

            /**
             * @see Expr
             */
            virtual std::string getType(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder);
};

class Identifier : public Expr{
    public:
            std::string name;

            explicit Identifier();  // Constructor

            /**
             * Creates a new Identifier object
             * 
             * @param name The name of the Identifier
             * 
             * @returns a new Identifier object.
             */
            explicit Identifier(const std::string& name);

            /**
             * @see Expr
             */
            virtual std::string print();

            /**
             * @see Expr
             */
            virtual std::string getType(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder);
};

class Integer : public Expr{
    public:
            int id;

            explicit Integer(); // Constructor

            /**
             * Creates a new Integer object
             * 
             * @param id The value of the Integer
             * 
             * @returns a new Integer object.
             */
            explicit Integer(int id);

            /**
             * @see Expr
             */
            virtual std::string print();

            /**
             * @see Expr
             */
            virtual std::string getType(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder);
};

class Self : public Identifier{
    public:
            explicit Self();    // Constructor

            /**
             * @see Expr
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder);
};

class String : public Expr{
    public:
            std::string name;

            explicit String();  // Constructor
            
            /**
             * Creates a new String object
             * 
             * @param name The name of the String
             * 
             * @returns a new String object.
             */
            explicit String(const std::string& name);

            /**
             * @see Expr
             */
            virtual std::string print();

            /**
             * @see Expr
             */
            virtual std::string getType(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder);
};


class Unit : public Expr{
    public:
            explicit Unit();    // Constructor

            /**
             * @see Expr
             */
            virtual std::string print();

            /**
             * @see Expr
             */
            virtual std::string getType(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder);
};

class UnOp : public Expr{
    public:
            enum Value {NOT, MINUS, ISNULL};
            Value value;
            std::shared_ptr<Expr> expr;

            explicit UnOp();    // Constructor

            /**
             * Creates a new UnOp object
             * 
             * @param value The type of operation of the UnOp
             * @param expr The right hand side of the UnOp
             * 
             * @returns a new UnOp object.
             */
            explicit UnOp(Value value, std::shared_ptr<Expr> expr);
            explicit UnOp(Value value, Expr* expr);

            /**
             * @see Expr
             */
            virtual std::string print();

            /**
             * @see Expr
             */
            virtual std::string getType(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual void semanticAnalysis(VSOPProgram& prog, SymbolTable& scope);

            /**
             * @see Expr
             */
            virtual llvm::Value* codegen_aux(VSOPProgram& prog, CodeGenerator& coder);
};

// Utils

/**
 * Determines whether a type is primitive or not.
 *  
 * @param var The type
 * 
 * @returns true if var is primitive, false else.
 */
bool is_primitive(const std::string& var);

/**
 * Determines whether a type is a class or not.
 * 
 * @param var The type
 * 
 * @returns true if var is primitve, false else.
 */
bool _is_class(const std::string& var, VSOPProgram& prog);

/**
 * Determines whether type_1 inherits from type_2
 * 
 * @param type_1 The supposed subtype
 * @param type_2 The supposed ancestor type
 * 
 * @returns true if type_1 inherits from type_2, false else
 */
bool inherits_from(VSOPProgram& prog, const std::string& type_1, const std::string& type_2);

/**
 * Determines the common parent of two types
 * 
 * @param type_1 The first type
 * @param type_2 The second type
 * 
 * @returns The common parent of type_1 and type_2
 */
std::string common_parent(VSOPProgram& prog, const std::string& type_1, const std::string& type_2);

#endif