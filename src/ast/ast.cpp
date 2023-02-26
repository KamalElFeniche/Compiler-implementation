#include "ast.hpp"
#include <sstream>
#include <iomanip>

using namespace std;




// Utils

bool is_primitive(const string& var){

    return var == "int32" || var == "bool" || var == "string" || var == "unit";
}

bool _is_class(const string& var, VSOPProgram& prog){

    if(prog.class_table.find(var) == prog.class_table.end())
        return false;
    else
        return true;
}

bool inherits_from(VSOPProgram& prog, const std::string& type_1, const std::string& type_2){

    if (!_is_class(type_1, prog)){

        if (type_1 == type_2)
            return true;
        else
            return false;

    }else{

        for(auto _class = prog.class_table[type_1]; _class != nullptr; _class = _class->parent_class)
            if (_class->name == type_2)
                return true;

        if (type_1 == type_2)
            return true;
        else   
            return false;
    }
}

string common_parent(VSOPProgram& prog, const std::string& type_1, const std::string& type_2){

    auto _class_1 = prog.class_table[type_1];
    auto _class_2 = prog.class_table[type_2];

    while(_class_1 != nullptr){
        if (inherits_from(prog, _class_2->name, _class_1->name))
            break;
        _class_1 = _class_1->parent_class;
    }

    return _class_1->name;
}

static llvm::Value* cast_to_target(VSOPProgram& prog, CodeGenerator& coder, llvm::Value* value, llvm::Type* target_type){


    llvm::Type* value_type = nullptr;

    if (value != nullptr)
        value_type =  value->getType();

    if (is_same_as(value_type, target_type))
        return value;

    if (inherits_from(prog, type_to_string(value_type), type_to_string(target_type)))
        return coder.builder->CreatePointerCast(value, target_type);

    return nullptr;

}

// Assign class
Assign::Assign(){}

Assign::Assign(const string& name, shared_ptr<Expr> expr): name(name), expr(expr) {}

Assign::Assign(const string& name, Expr* expr): name(name), expr(expr) {}

string Assign::print(){
    string text = "Assign(" + name + "," + expr->print() + ")";

    if (_type != "")
        text += ":" + _type;

    return text;
}

string Assign::getType(VSOPProgram& prog, SymbolTable& scope){
    
    if (scope.look_up(name))
        _type = scope.get(name);
    else
        _type = "unknown";

    return _type;
}

void Assign::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){

    expr->semanticAnalysis(prog, scope);
    string type_expr = expr->getType(prog, scope);
    
    if (scope.look_up(name)){

        string type_assign = scope.get(name);
        if (! inherits_from(prog, type_expr, type_assign)){
            semanticError("expected type " + type_assign + " but got type " + type_expr);
            prog.nb_errors++;
        }

    }else{

        semanticError("trying to assign to undefined " + name + " variable");
        prog.nb_errors++;
        
    }

    getType(prog, scope);   // Call to getType just to set the type of the Assign
}

llvm::Value* Assign::codegen_aux(VSOPProgram& prog, CodeGenerator& coder){
    expr->codegen(prog, coder);

    llvm::Value* self_value = coder.get_val("self");
    shared_ptr<Class> _class;
    if (self_value != nullptr)  // Means that we are assigning to a field of Self !
        _class = prog.class_table[type_to_string(self_value->getType())];
    else
        _class = nullptr;

    llvm::Type* target_type = nullptr;

    if (coder.look_up(name)){
        target_type = coder.get_type(name);

    }else if (_class != nullptr && _class->field_table.find(name) != _class->field_table.end()){

        target_type = _class->field_table[name]->get_llvm_type();   // Get the field type

    }else{

        return nullptr;
    }

    llvm::Value* casted_value = nullptr;

    if ( (! is_unit(expr->get_llvm_type())) || (!is_unit(target_type))){
        casted_value = cast_to_target(prog, coder, expr->expr_value, target_type);
        if (casted_value == nullptr)
            return nullptr;
    }

    if (coder.look_up(name))
        coder.store(name, casted_value);

    else if (! is_unit(target_type)){
        // Store the new value of the field
        coder.builder->CreateStore(casted_value, 
                                coder.builder->CreateStructGEP(
                                    self_value, 
                                    _class->field_table[name]->index_vtable
                                )
                        );
    }

    return casted_value;

}

// BinOp class
BinOp::BinOp(){}

BinOp::BinOp(Value value, shared_ptr<Expr> left, shared_ptr<Expr> right): value(value), left(left), right(right) {}

BinOp::BinOp(Value value, Expr* left, Expr* right): value(value), left(left), right(right) {}

string BinOp::print(){
    string text = "BinOp(";

    switch (value){
        case EQUAL: text += "=,"; break;
        case LOWER: text += "<,"; break;
        case LOWER_EQ: text += "<=,"; break;
        case PLUS: text += "+,"; break;
        case MINUS: text += "-,"; break;
        case TIMES: text += "*,"; break;
        case DIV: text += "/,"; break;
        case POW: text += "^,"; break;
        case AND: text += "and,"; break;
    }

    text += left->print() + "," + right->print() + ")";
    if (_type != "")
        text += ":" + _type;
    return text;
}

string BinOp::getType(VSOPProgram& prog, SymbolTable& scope){
    switch (value){
        case EQUAL: _type = "bool"; return "bool";
        case LOWER: _type = "bool"; return "bool";
        case LOWER_EQ: _type = "bool"; return "bool";
        case PLUS: _type = "int32"; return "int32";
        case MINUS:_type = "int32"; return "int32";
        case TIMES: _type = "int32"; return "int32";
        case DIV: _type = "int32"; return "int32";
        case POW: _type = "int32"; return "int32";
        case AND: _type = "bool"; return "bool";
    }
}

void BinOp::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){

    string expected_type;
    switch(value){
        case EQUAL: expected_type = ""; break;
        case LOWER: expected_type = "int32"; break;
        case LOWER_EQ: expected_type = "int32"; break;
        case PLUS: expected_type = "int32"; break;
        case MINUS:expected_type = "int32"; break;
        case TIMES: expected_type = "int32"; break;
        case DIV: expected_type = "int32"; break;
        case POW: expected_type = "int32"; break;
        case AND: expected_type = "bool"; break;
    }

    

    left->semanticAnalysis(prog, scope);
    string left_type = left->getType(prog, scope);
    right->semanticAnalysis(prog, scope);
    string right_type = right->getType(prog, scope);

    if (expected_type != ""){ // Not in the case of an equality check
        if (right_type != left_type){
            semanticError("both type must be the same to use a binary operation");
            prog.nb_errors++;
        }

        if (left_type != expected_type){
            semanticError("expected type " + expected_type + ", but got type " + left_type);
            prog.nb_errors++;
        }

        if (right_type != expected_type){
            semanticError("expected type " + expected_type + ", but got type " + right_type);
            prog.nb_errors++;
        }

    }else{
        // Here we are in the case of an equality check
        if (is_primitive(left_type) || is_primitive(right_type)){

            if (right_type != left_type){
                semanticError("both type must be the same to use a binary operation");
                prog.nb_errors++;
            }

        }
    }

    getType(prog, scope);   // Set the type of the BinOp

}

llvm::Value* BinOp::codegen_aux(VSOPProgram& prog, CodeGenerator& coder){
    

    if (value == AND){
        /**
         * Since in VSOP the "and" operator is shortcircuited,
         * then one can see "a && b" as if a then b else false
         */
        return If(left, right, make_shared<Boolean>(false)).codegen_aux(prog, coder);
    }
    
    /** For the other cases, we just generate the llvm value for the rhs and lhs
     * then, we apply a function inside the builder
     */

    left->codegen(prog, coder);
    right->codegen(prog, coder);

    if (value == LOWER){
        return coder.builder->CreateICmpSLT(left->expr_value, right->expr_value);
    }

    if (value == LOWER_EQ){
        return coder.builder->CreateICmpSLE(left->expr_value, right->expr_value);
    }

    if (value == PLUS){
        return coder.builder->CreateAdd(left->expr_value, right->expr_value);
    }

    if (value == MINUS){
        return coder.builder->CreateSub(left->expr_value, right->expr_value);
    }

    if (value == TIMES){
        return coder.builder->CreateMul(left->expr_value, right->expr_value);
    }

    if (value == DIV){
        return coder.builder->CreateSDiv(left->expr_value, right->expr_value);
    }

    if (value == POW){
        return coder.builder->CreateFPToSI(
                coder.builder->CreateCall(
                        coder.module->getOrInsertFunction(
                                "llvm.powi.f64",
                                llvm::FunctionType::get(
                                        llvm::Type::getDoubleTy(*coder.context),
                                            {
                                                llvm::Type::getDoubleTy(*coder.context),
                                                coder.to_type("int32"),
                                            },
                                            false
                                )
                        ),
                        {
                            coder.builder->CreateSIToFP(
                                left->expr_value,
                                llvm::Type::getDoubleTy(*coder.context)
                            ),
                            right->expr_value
                        }
                ),
                coder.to_type("int32")
        );
    }

    if (value == EQUAL){

        llvm::Type* left_type = left->get_llvm_type();
        llvm::Type* right_type = right->get_llvm_type();

        if (is_same_as(left_type, right_type)){
            
            if (is_string(left_type)){

                llvm::Value* comp = coder.builder->CreateCall(
                    coder.module->getOrInsertFunction(
                            "strcmp",
                            llvm::FunctionType::get(
                                    coder.to_type("int32"),
                                    {
                                        llvm::Type::getInt8PtrTy(*coder.context),
                                        llvm::Type::getInt8PtrTy(*coder.context),
                                    },
                                    false
                            )
                    ),  // Call the strcmp function and apply it to lhs and rhs
                    {left->expr_value, right->expr_value}
                );
                // Then compare its value to the default int32 value 
                return coder.builder->CreateICmpEQ(comp, coder.default_val("int32"));

            } else if (is_unit(left_type)){
                // Always true, since unit has only one value
                return llvm::ConstantInt::getTrue(*coder.context);

            }else{
                // Here we are in the case of integers
                return coder.builder->CreateICmpEQ(left->expr_value, right->expr_value);

            }
        } else if (is__class(left_type) && is__class(right_type)){
            // Determine first their common ancestor
            llvm::Type* ancestor_type = prog.class_table[common_parent(prog, type_to_string(left_type),type_to_string(right_type))]->get_type(coder)->getPointerTo();
            
            // Then check the equlity when they have been casted to their common ancestor type
            return coder.builder->CreateICmpEQ(
                                cast_to_target(prog, coder, left->expr_value, ancestor_type),
                                cast_to_target(prog, coder, right->expr_value, ancestor_type)
            );
        } else {
            
            return coder.default_val("bool"); // SHould not reach here because Semantic analysis was already done
        }
    }

    return coder.default_val("int32"); // Should never reach here, but the compiler (gcc) complains about non void function not returning a value
}

// Block class

Block::Block() {}

Block::Block(const VSOPList<Expr>& expr): expr(expr.list) {}

string Block::print(){
    string text = expr.print();
    if (_type != "")
        text += ":" + _type;

    return text; 
}

string Block::getType(VSOPProgram& prog, SymbolTable& scope){
    if (expr.list.empty())
        _type = "unknown";
    else
        _type = expr.list.back()->getType(prog, scope);
    
    return _type;
}

void Block::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){

    expr.semanticAnalysis(prog, scope);
    getType(prog, scope);
}

llvm::Value* Block::codegen_aux(VSOPProgram& prog, CodeGenerator& coder){
    expr.codegen(prog, coder);

    if (expr.list.empty())
        return nullptr;
    else
        return expr.list.back()->expr_value;
}

// Boolean Class
Boolean::Boolean(){}

Boolean::Boolean(bool boolean): boolean(boolean) {}

string Boolean::print(){
    std::string text;
    if (boolean)
        text = "true";
    else
        text = "false";

    if (_type != "")
        text += ":" + _type;
    
    return text;
}

string Boolean::getType(VSOPProgram& prog, SymbolTable& scope){
    _type = "bool";
    return _type;
}

void Boolean::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){
    getType(prog, scope); // Just set the type and that's it
   
}

llvm::Value* Boolean::codegen_aux(VSOPProgram& prog, CodeGenerator& coder){
    return llvm::ConstantInt::get(coder.to_type("bool"), boolean);
}

// Call class

Call::Call(){}

Call::Call(shared_ptr<Expr> obj, const string& name, const VSOPList<Expr>& arguments): obj(obj), name(name), arguments(arguments.list) {}

Call::Call(Expr* obj, const string& name, const VSOPList<Expr>& arguments): obj(obj), name(name), arguments(arguments.list) {}

string Call::print(){
    string text = "Call(" + obj->print() + "," + name + "," + arguments.print() + ")";
    if (_type != "")
        text += ":" + _type;
    return text;
}

string Call::getType(VSOPProgram& prog, SymbolTable& scope){
    
    string scope_type = obj->getType(prog, scope);
    bool control = false;

    if (_is_class(scope_type, prog)){

        auto _class = prog.class_table[scope_type];
        while(_class != nullptr){

            if (_class->method_table.find(name) != _class->method_table.end()){
                control = true; // The class method is found !
                break;
            }

            _class = _class->parent_class;
            
        }

        if (control){
            _type = _class->method_table[name]->return_type; 
            return _type;
        }
    }

    // Means that we have tried to call to something which was not a class -> error
    _type = "unknown";
    return _type;
}

void Call::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){

    // Perfoms semantic on the object and get its type
    obj->semanticAnalysis(prog, scope);
    string obj_type = obj->getType(prog, scope);

    bool control = false;

    arguments.semanticAnalysis(prog, scope);

    if (_is_class(obj_type, prog)){
        auto _class = prog.class_table[obj_type];

        auto it = _class;

        while (it != nullptr){

            if (it->method_table.find(name) != it->method_table.end()){
                control = true; // Method found
                break;
            }

            it = it->parent_class;
        }

        if (control){
            auto _method = it->method_table[name];
            // Now check if it is called with the right number of arguments
            if (arguments.list.size() != _method->formal.list.size()){
                semanticError("wrong number of arguments to call function " + _method->name);
                prog.nb_errors++;
            }

            else{

                for (int i = 0; i < arguments.list.size(); i++)
                    // Now check the return type
                    if (! inherits_from(prog, arguments.list[i]->getType(prog, scope), _method->formal.list[i]->getType(prog, scope))){
                        semanticError("expected type " + _method->formal.list[i]->type + " but received type " + arguments.list[i]->getType(prog, scope));
                        prog.nb_errors++;
                    }

            } 

        }else{
            semanticError("undefined method " + name); 
            prog.nb_errors++;
        }

    }else{
        semanticError(obj_type + " is not a class");
        prog.nb_errors++;
    }

    getType(prog, scope);
}

llvm::Value* Call::codegen_aux(VSOPProgram& prog, CodeGenerator& coder){
    
    obj->codegen(prog, coder);
    llvm::Type* scope_type = obj->get_llvm_type();

    arguments.codegen(prog, coder);

    shared_ptr<Method> method;
    llvm::Function* function = nullptr;
    vector<llvm::Value*> params;
    llvm::Value* obj_value = nullptr;

    if (is_unit(scope_type) || is__class(scope_type) || coder.look_up("self")){

        if (is_unit(scope_type)){   // here we are in the case some_method(param_1, ...)
            obj_value = coder.get_val("self");
        }else{
            obj_value = obj->expr_value;    //Here we are in the case obj.some_method(param_1, ...)
        }

        // Retrieve the class
        shared_ptr<Class> _class = prog.class_table[type_to_string(obj_value->getType())];

        method = _class->method_table[name];

        function = (llvm::Function*) coder.builder->CreateLoad(     // Load the method
                                    coder.builder->CreateStructGEP( // Get the method
                                        coder.builder->CreateLoad(  // Load vtable
                                            coder.builder->CreateStructGEP(obj_value, 0) // Get vtable
                                        ),
                                        method->index_vtable // Index of method in vtable
                                                
                                    )
        );

        params.push_back(obj_value); // Push self as first argument
    }

    if (function != nullptr){

        for (int i = 0; i < arguments.list.size(); i++){

            if (!is_unit(arguments.list[i]->get_llvm_type()) || method->formal.list[i]->type != "unit"){
                // Cast the value for dynamic dispatch and if the type is != unit
                llvm::Value* casted_value = cast_to_target(prog, coder, arguments.list[i]->expr_value, coder.to_type(method->formal.list[i]->type));
                params.push_back(casted_value);
            }
        }

        // Call the method
        return coder.builder->CreateCall(function, params); 
    }

    return nullptr;
}

// Class class

Class::Class(){}

Class::Class(const string& name, const string& parent, const VSOPList<Field>& field, const VSOPList<Method>& method): name(name), parent(parent), field(field.list), method(method.list){}

string Class::print(){
    string text = "Class(" + name + "," + parent + "," + field.print() + "," + method.print() + ")";
    return text;
}

void Class::declaration(VSOPProgram& prog){
    // Declaration of the fields of the class
    auto it = field.list.begin();
    while(it != field.list.end()){

        if (field_table.find((*it)->name) == field_table.end()){ // Field does not exist yet, we must add it to the table
            
            field_table[(*it)->name] = *it;
            it++;

        }else{

            semanticError("redefinition of field " + (*it)->name + " of class " + name);
            prog.nb_errors++;
            it = field.list.erase(it);
        }
    }

    // Declaration of the methods of the class
    auto _it = method.list.begin();
    while(_it != method.list.end()){

        if (method_table.find((*_it)->name) == method_table.end()){ // Method does not exist yet, so we must add it
            
            method_table[(*_it)->name] = *_it;
            (*_it)->declaration(prog);
            _it++;

        }else{

            semanticError("redefinition of method " + (*_it)->name + " of class " + name);
            prog.nb_errors++;
            _it = method.list.erase(_it); // Just erase the redifined method 
        }
    }

    
}

void Class::override(VSOPProgram& prog){

    // Check first if we do not override some fields
    bool control = false;
    auto it = field.list.begin();
    
    while(it != field.list.end()){

        auto _parent = parent_class;

        while(_parent != NULL){

            if (_parent->field_table.find((*it)->name) != _parent->field_table.end()){
                
                semanticError("overriding field " + _parent->field_table[(*it)->name]->name + " in class " + name);
                field_table.erase((*it)->name);
                it = field.list.erase(it);
                prog.nb_errors++;
                control = true;
                break;
            
            }else{

                _parent = _parent->parent_class;
            }
        }

        if (control){

            control = false;

        }else{

            it++;
        }
    }

    // Then, check if we do not override some methods
    control = false;
    auto iter = method.list.begin();

    while(iter != method.list.end()){

        auto _parent = parent_class;

        while(_parent != NULL){

            if(_parent->method_table.find((*iter)->name) != _parent->method_table.end()){
                // Method found in parent
                auto _method = _parent->method_table[(*iter)->name];

                //Check return type
                if ((*iter)->return_type != _method->return_type){

                    control = true;
                    semanticError("overriding method " + (*iter)->name + " with different return type");
                    prog.nb_errors++;
                    break;

                }
                // Check the number of formals
                else if((*iter)->formal.list.size() != _method->formal.list.size()){

                    control = true;
                    semanticError("overriding method " + (*iter)->name + " with different number of formals");
                    prog.nb_errors++;
                    break;

                }else{
                    // Check the types of the formals
                    for (int i = 0; i < _method->formal.list.size(); i++)
                        if((*iter)->formal.list[i]->type != _method->formal.list[i]->type){
                            semanticError("overriding method " + (*iter)->name + " with different formal type");
                            prog.nb_errors++;
                            control = true;
                            break;
                        }

                }
                
            }

            _parent = _parent->parent_class;
        }

        if (control){

            control = false;
            method_table.erase((*iter)->name);
            iter = method.list.erase(iter);

        }else{

            iter++;
        }
    }
}

void Class::enter_scope(SymbolTable& scope){
    // Add all the fields in the symbol table
    field.enter_scope(scope);
    // Also enter the scope of the parent class
    if (parent_class != nullptr){
        parent_class->enter_scope(scope);
    }
    // Now self refers to this class
    scope.insert("self", name);
}

void Class::exit_scope(SymbolTable& scope){
    // First remove self
    scope.remove("self");
    //Remove the scope of the parents
    if (parent_class != nullptr)
        parent_class->exit_scope(scope);
    // Remove all the fields
    field.exit_scope(scope);
}

void Class::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){
    
    field.semanticAnalysis(prog, scope);
    enter_scope(scope);
    method.semanticAnalysis(prog, scope);
    exit_scope(scope);
}

string Class::getType(VSOPProgram& prog, SymbolTable& scope){
    return name;
}

string Class::struct_name(){
    return "struct." + name;
}

bool Class::is_declared(CodeGenerator& coder){
    return coder.module->getFunction(name + "__new");
}

llvm::StructType* Class::get_type(CodeGenerator& coder){
    // Get the type with the name of the class
    llvm::StructType* class_struct = coder.module->getTypeByName(this->struct_name());
    if (class_struct != nullptr)
        return class_struct;
    else
        return llvm::StructType::create(*coder.context, this->struct_name()); // Create it if it does not exists
}

void Class::pre_codegen(VSOPProgram& prog, CodeGenerator& coder){
    
    if (parent_class != nullptr && ! parent_class->is_declared(coder))
        parent_class->pre_codegen(prog, coder);

    int field_index = 1; // index of field in vtable (start at 1 because first element is vtable)
    int method_index = 0; // index of method in vtable

    if (parent_class != nullptr){
        for (auto& it : parent_class->field_table)
            field_index = max(field_index, it.second->index_vtable + 1);  // Field_index = max_{i} parent_field_i

        for(auto& it : parent_class->method_table)
            method_index = max(method_index, it.second->index_vtable + 1); // method_index = max_{i} parent_method_i
    }

    for (auto& it : field.list){
        if (it->type == "unit") // Unit type not stored so do not increment the index
            it->index_vtable = field_index;
        else
            it->index_vtable = field_index++;

    }

    if (parent_class != nullptr)    // Insert all the fields of the parent
        field_table.insert(parent_class->field_table.begin(), parent_class->field_table.end());

    for (auto& it : method.list){
        it->parent = this;
        it->pre_codegen(prog, coder);   // Declare the method in the module
        llvm::Function* f = it->get_function(coder);
        
        if (parent_class != nullptr && parent_class->method_table.find(it->name) != parent_class->method_table.end()){
            auto _method = parent_class->method_table[it->name];

            it->index_vtable = _method->index_vtable;   // method is inherited -> just retrieve the method index
        }else{
            it->index_vtable = method_index++;
        }
    }

    // Now add the methods of the parent that do not have the same name of one of the child method
    if (parent_class != nullptr)
        for (auto it = parent_class->method_table.begin(); it != parent_class->method_table.end(); it++)
            if (method_table.find(it->first) == method_table.end())
                method_table[it->first] = it->second;

    // Initialize the structure that represents the class and the vtable
    llvm::StructType* self_type = this->get_type(coder);
    llvm::StructType* vtable_type = llvm::StructType::create(*coder.context, this->struct_name() + "Vtable");

    // This vector represents the structure of the class
    vector<llvm::Type*> elements_type;

    elements_type.push_back(vtable_type->getPointerTo());   // Insert vtable

    for (auto& it : field_table){

        llvm::Type* field_type = coder.to_type(it.second->type);

        if (it.second->type == "unit")  // If unit do not insert the field
            continue;

        if (it.second->index_vtable >= elements_type.size())
            elements_type.resize(it.second->index_vtable + 1);

        elements_type[it.second->index_vtable] = field_type; // Insert field at correct position
    }

    // Create the body of struct
    self_type->setBody(elements_type);

    vector<llvm::Constant*> elements;
    elements_type.clear();

    for (auto& it : method_table){

        if (it.second->index_vtable >= elements_type.size()){
            elements_type.resize(it.second->index_vtable + 1);
            elements.resize(it.second->index_vtable + 1);
        }

        llvm::Function* function = it.second->get_function(coder); // Retrieve the function

        llvm::Type* return_type = function->getReturnType();
        vector<llvm::Type*> params_type = function->getFunctionType()->params();    // get the parameters of the function
        params_type[0] = self_type->getPointerTo(); // First arguments of all class methods is self!

        llvm::FunctionType* function_type = llvm::FunctionType::get(return_type, params_type, false);
        
        // Cast the function the correct type
        elements[it.second->index_vtable] = (llvm::Constant*) coder.builder->CreatePointerCast(function, function_type->getPointerTo());
        elements_type[it.second->index_vtable] = function_type->getPointerTo();
    }
    // create the body of the vtable
    vtable_type->setBody(elements_type);

    // Create new global variable which represents the class
    llvm::GlobalVariable* vtable = new llvm::GlobalVariable(*coder.module, vtable_type, true, llvm::GlobalVariable::InternalLinkage, llvm::ConstantStruct::get(vtable_type, elements), "vtable." + name);

    // Create the "new" functoin which has global visibility -> externalLinkage
    llvm::FunctionType* function_type = llvm::FunctionType::get(self_type->getPointerTo(), false);
    llvm::Function* function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, name + "__new", *coder.module);

    // Same here for the "init" method
    function_type = llvm::FunctionType::get(llvm::Type::getVoidTy(*coder.context), {self_type->getPointerTo()}, false);
    function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, name + "__init", *coder.module);
    function->arg_begin()->setName("self"); // arg of init is always "self"
}

void Class::codegen(VSOPProgram& prog, CodeGenerator& coder){
    
    // Retrieve the "init" function
    llvm::Function* function = coder.module->getFunction(name + "__init");
    // Create the block which will represent the "body" of the function. It is a container of instructions
    // that executes sequentially
    llvm::BasicBlock* entry_point = llvm::BasicBlock::Create(*coder.context, "", function);
    coder.builder->SetInsertPoint(entry_point);

    // If there is a parent, call it's initializer
    if (parent_class != nullptr){
        coder.builder->CreateCall(coder.module->getFunction(parent + "__init"), 
                            {coder.builder->CreatePointerCast(
                                            function->arg_begin(), 
                                            parent_class->get_type(coder)->getPointerTo())
                            }
        );
    }

    for (auto& it : field.list){
        it->codegen(prog, coder); //Generate the code for the field and set its SSA value

        if (it->type != "unit") // Store only if != unit
            coder.builder->CreateStore(it->expr_value, // store its SSA value
                                coder.builder->CreateStructGEP(
                                    function->arg_begin(), 
                                    field_table[it->name]->index_vtable
                                )
            );
    }

    // init is of return type void
    coder.builder->CreateRetVoid();

    // Get the "new" method
    function = coder.module->getFunction(name + "__new");

    // Create its block
    entry_point = llvm::BasicBlock::Create(*coder.context, "", function);
    llvm::BasicBlock* init_block = llvm::BasicBlock::Create(*coder.context, "init", function);
    llvm::BasicBlock* null_block = llvm::BasicBlock::Create(*coder.context, "null", function);

    // This specifies that created instructions should be appened to the end of this block
    coder.builder->SetInsertPoint(entry_point);

    // Computes the size of the structure we need
    size_t size = coder.module->getDataLayout().getTypeAllocSize(this->get_type(coder));
    llvm::Value* mem = coder.builder->CreateCall(
            coder.module->getOrInsertFunction(
                    "malloc", 
                    llvm::FunctionType::get(
                            llvm::Type::getInt8PtrTy(*coder.context), 
                            {llvm::Type::getInt64Ty(*coder.context)}, 
                            false
                    )
            ),
            {llvm::ConstantInt::get(llvm::Type::getInt64Ty(*coder.context), size)}
    );

    // Conditional branching
    coder.builder->CreateCondBr(coder.builder->CreateIsNull(mem), null_block, init_block);

    // init block
    coder.builder->SetInsertPoint(init_block);

    llvm::Value* instance = coder.builder->CreateBitCast(mem, this->get_type(coder)->getPointerTo());

    coder.builder->CreateCall(coder.module->getFunction(name + "__init"), {instance});

    coder.builder->CreateStore(coder.module->getNamedValue("vtable." + name), coder.builder->CreateStructGEP(instance, 0));

    coder.builder->CreateRet(instance);

    // Null block
    coder.builder->SetInsertPoint(null_block);
    coder.builder->CreateRet(llvm::ConstantPointerNull::get(this->get_type(coder)->getPointerTo()));

    // Generate code for the methods
    method.codegen(prog, coder);
}

// Expr class

void Expr::codegen(VSOPProgram& prog, CodeGenerator& coder){
    expr_value = this->codegen_aux(prog, coder);
}

llvm::Type* Expr::get_llvm_type(){
    if (expr_value == nullptr)
        return nullptr;
    else
        return expr_value->getType();
}

llvm::Value* Expr::get_value(){
    return expr_value;
}

// Field class

Field::Field(){}

Field::Field(const string& name, const string& type, shared_ptr<Expr> init): name(name), type(type), init(init) {}

Field::Field(const string& name, const string& type, Expr* init): name(name), type(type), init(init) {}

string Field::print(){
    string text = "Field(" + name + "," + type;
    if (init)
        text += "," + init->print();

    return text + ")";  
}

string Field::getType(VSOPProgram& prog, SymbolTable& scope){
    _type = type;
    return type;
}

void Field::enter_scope(SymbolTable& scope){
    
    scope.insert(name, type);
}

void Field::exit_scope(SymbolTable& scope){
    scope.remove(name);
}

void Field::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){

    if (!is_primitive(type) && !_is_class(type, prog)){
        semanticError("unknown type " + type);
        prog.nb_errors++;
    }

    if (init != nullptr){

        init->semanticAnalysis(prog, scope);
        string type_init = init->getType(prog, scope);
        
        if (!inherits_from(prog, type_init, type)){
            semanticError("got type " + type_init + ", but expected type " + type);
            prog.nb_errors++;
        }
    }

    getType(prog, scope);
}

llvm::Value* Field::codegen_aux(VSOPProgram& prog, CodeGenerator& coder){
    
    llvm::Type* field_type = coder.to_type(type);

    // If no initializer then default value
    if (init == nullptr)
        return coder.default_val(field_type);

    init->codegen(prog, coder);

    if (is_unit(init->get_llvm_type()) && is_unit(field_type))
        return nullptr;

    llvm::Value* casted_value = cast_to_target(prog, coder, init->expr_value, field_type);

    return casted_value;

}

// Formal class

Formal::Formal() {};

Formal::Formal(const string& name, const string& type): name(name), type(type) {}

string Formal::print(){
    return name + ":" + type;
}

void Formal::enter_scope(SymbolTable& scope){
    scope.insert(name, type);
}

void Formal::exit_scope(SymbolTable& scope){
    scope.remove(name);
}

void Formal::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){
    if (!is_primitive(type) && !_is_class(type, prog)){
        semanticError("unknown type " + type);
        prog.nb_errors++;
    }
}

string Formal::getType(VSOPProgram& prog, SymbolTable& scope){

    return type;
}

// Identifier class

Identifier::Identifier(){}

Identifier::Identifier(const string& name): name(name) {}

string Identifier::print(){
    string text = name;
    if (_type != "")
        text += ":" + _type;

    return text;
}

string Identifier::getType(VSOPProgram& prog, SymbolTable& scope){
    
    if (scope.look_up(name)){
        _type = scope.get(name);
    }else{
        _type = "unknown";
    }

    return _type;
}

void Identifier::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){
    if (! scope.look_up(name)){
        semanticError("undefined identifier: " + name);
        prog.nb_errors++;
    }
    getType(prog, scope);
}

llvm::Value* Identifier::codegen_aux(VSOPProgram& prog, CodeGenerator& coder){
    
    // Check if the name of the identifier is in the symbol table
    if (coder.look_up(name))
        return coder.load(name);

    // Else it is a field of self
    llvm::Value* self = coder.get_val("self");
    shared_ptr<Class> _class;

    if (self != nullptr){
        _class = prog.class_table[type_to_string(self->getType())];

    }else{

        _class = nullptr;
    }

    if (_class != nullptr){
        if (! is_unit(_class->field_table[name]->get_llvm_type()))
            
            return coder.builder->CreateLoad(   // load the field
                        coder.builder->CreateStructGEP( // Get pointer to the field
                            self,
                            _class->field_table[name]->index_vtable // field index in vtable
                        )
            );

        else
            return nullptr;
    }

    return nullptr;
}

// If class
If::If(){}

If::If(shared_ptr<Expr> cond, shared_ptr<Expr> then, shared_ptr<Expr> else_expr): cond(cond), then(then), else_expr(else_expr){}

If::If(Expr* cond, Expr* then, Expr* else_expr): cond(cond), then(then), else_expr(else_expr) {}

string If::print(){

    string text = "If(" + cond->print() + "," + then->print();
    if (else_expr)
        text += "," + else_expr->print();
    text += ")";

    if (_type != "")
        text += ":" + _type;

    return text;
}

string If::getType(VSOPProgram& prog, SymbolTable& scope){
    
    string type_then = then->getType(prog, scope);
    string type_else;

    if (else_expr != nullptr)
        type_else = else_expr->getType(prog, scope);
    else
        type_else = "unit";

    if (type_else == "unit" || type_then == "unit"){

        _type = "unit";
        return _type;
    }
    else if (is_primitive(type_then) && type_then == type_else){

        _type = type_then;
        return _type;
    }
    else if (_is_class(type_then, prog) && _is_class(type_else, prog)){
        _type = common_parent(prog, type_then, type_else);
        return _type;
    }
    _type = "unknown";
    return _type;

}

void If::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){

    cond->semanticAnalysis(prog, scope);

    if (cond->getType(prog, scope) != "bool"){
        semanticError("condition must have type bool");
        prog.nb_errors++;
    }

    then->semanticAnalysis(prog, scope);

    if (else_expr != nullptr)
        else_expr->semanticAnalysis(prog, scope);

    if (getType(prog, scope) == "unknown"){
        semanticError("types of the condition do not agree");
        prog.nb_errors++;
    }

    getType(prog, scope);
}

llvm::Value* If::codegen_aux(VSOPProgram& prog, CodeGenerator& coder){
    
    cond->codegen(prog, coder);

    // Get the current function that is built
    llvm::Function* function = coder.builder->GetInsertBlock()->getParent();

    llvm::BasicBlock* then_block = llvm::BasicBlock::Create(*coder.context, "then", function);
    llvm::BasicBlock* else_block = llvm::BasicBlock::Create(*coder.context, "else", function);
    llvm::BasicBlock* end_block = llvm::BasicBlock::Create(*coder.context, "end", function);

    // Create the conditional branching
    coder.builder->CreateCondBr(
            cond->expr_value,
            then_block,
            else_block
    );

    // Emit then value
    coder.builder->SetInsertPoint(then_block);
    then->codegen(prog, coder);
    llvm::BasicBlock* then_block_aux = coder.builder->GetInsertBlock();

    // Emit else value
    coder.builder->SetInsertPoint(else_block);
    if (else_expr != nullptr)
        else_expr->codegen(prog, coder);

    llvm::BasicBlock* else_block_aux = coder.builder->GetInsertBlock();

    llvm::Value* then_value = then->expr_value;

    llvm::Value* else_value;

    if (else_expr == nullptr)
        else_value = nullptr;
    else
        else_value = else_expr->expr_value;

    llvm::Type* then_type = then->get_llvm_type();
    llvm::Type* else_type = else_value ? else_value->getType() : nullptr;
    llvm::Type* end_type = nullptr;

    if (is_same_as(then_type, else_type))
        end_type = then_type;

    else if (is__class(then_type) && is__class(else_type))
        end_type = prog.class_table[common_parent(prog, type_to_string(then_type), type_to_string(else_type))]->get_type(coder)->getPointerTo();

    coder.builder->SetInsertPoint(then_block_aux);

    // Cast if needed
    if (!is_unit(end_type) && ! is_same_as(then_type, end_type))
        then_value = cast_to_target(prog, coder, then_value, end_type);
    coder.builder->CreateBr(end_block);

    coder.builder->SetInsertPoint(else_block_aux);
    if (!is_unit(end_type) && ! is_same_as(else_type, end_type))
        else_value = cast_to_target(prog, coder, else_value, end_type);
    coder.builder->CreateBr(end_block);

    coder.builder->SetInsertPoint(end_block);

    if (is_unit(end_type))
        return nullptr;
    
    // Create the PHI instruction
    auto* phi = coder.builder->CreatePHI(end_type, 2);
    phi->addIncoming(then_value, then_block_aux);
    phi->addIncoming(else_value, else_block_aux);

    return phi;

}

// Integer class

Integer::Integer(){}

Integer::Integer(int id): id(id) {}

string Integer::print(){
    string text = to_string(id);
    if (_type != "")
        text += ":" + _type;

    return text;
}

string Integer::getType(VSOPProgram& prog, SymbolTable& scope){
    _type = "int32";
    return "int32";
}

void Integer::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){

    getType(prog, scope);
}

llvm::Value* Integer::codegen_aux(VSOPProgram& prog, CodeGenerator& coder){
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*coder.context), id);
}

// Let class
Let::Let(){}

Let::Let(const string& name, const string& type, shared_ptr<Expr> init, shared_ptr<Expr> scope): name(name), type(type), init(init), scope(scope) {}

Let::Let(const string& name, const string& type, Expr* init, Expr* scope): name(name), type(type), init(init), scope(scope) {}

string Let::print(){
    string text = "Let(" + name + "," + type;
    if (init)
        text += "," + init->print();
    text += "," + scope->print() + ")";

    if (_type != "")
        text += ":" + _type;

    return text;
}

string Let::getType(VSOPProgram& prog, SymbolTable& scope){
    enter_scope(scope);
    _type = this->scope->getType(prog ,scope);
    exit_scope(scope);
    return _type;

}

void Let::enter_scope(SymbolTable& scope){
    scope.insert(name, type);
}

void Let::exit_scope(SymbolTable& scope){
    scope.remove(name);
}

void Let::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){

    if (!is_primitive(type) && !_is_class(type, prog)){
        semanticError("unknown type: " + type);
        prog.nb_errors++;
    }

    if (init != nullptr){

        init->semanticAnalysis(prog, scope);
        string type_init = init->getType(prog, scope);

        if (!inherits_from(prog, type_init, type)){
            semanticError("expected type: " + type + " but received: " + type_init);
            prog.nb_errors++;
        }
    }

    enter_scope(scope);
    this->scope->semanticAnalysis(prog, scope);
    exit_scope(scope);

    getType(prog, scope);
}

llvm::Value* Let::codegen_aux(VSOPProgram& prog, CodeGenerator& coder){
    
    llvm::Type* let_type = coder.to_type(type);

    if (let_type != nullptr){
        llvm::Value* casted_value = nullptr;

        if (init != nullptr){

            init->codegen(prog, coder);

            if (!is_unit(init->get_llvm_type()) || !is_unit(let_type)){
                // Cast if needed
                casted_value = cast_to_target(prog, coder, init->expr_value, let_type);
            }
        }

        // If there is no init -> default value
        if (casted_value == nullptr)
            casted_value = coder.default_val(let_type);

        // Need to allocate memory to the new variable and store it
        coder.allocate(name, let_type);
        coder.store(name, casted_value);
    }

    scope->codegen(prog, coder);

    coder.remove(name);

    return scope->expr_value;
}

// Method class

Method::Method(){}

Method::Method(const string& name, const string& return_type, const VSOPList<Formal>& formal, shared_ptr<Block> block): name(name), return_type(return_type), formal(formal.list), block(block){}

Method::Method(const string& name, const string& return_type, const VSOPList<Formal>& formal, Block* block): name(name), return_type(return_type), formal(formal.list), block(block){}

string Method::print(){
    string text = "Method(" + name + "," +   formal.print() + "," + return_type;
    if (block != nullptr)
        text +=  "," + block->print();
    return text + ")";
}

void Method::declaration(VSOPProgram& prog){
    // Declare all the formals of the method, and check for redefinition

    auto it = formal.list.begin();
    while(it != formal.list.end()){
        
        if (formal_table.find((*it)->name) == formal_table.end()){
            
            formal_table[(*it)->name] = *it;
            it++;

        }else{

            semanticError("redefinition of formal " + (*it)->name + " of method " + name);
            it = formal.list.erase(it);
            prog.nb_errors++;
        }
    }

}

void Method::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){

    formal.semanticAnalysis(prog, scope);

    if (! _is_class(return_type, prog) && ! is_primitive(return_type)){
        semanticError("unknown type: " + return_type);
        prog.nb_errors++;
    }

    enter_scope(scope);
    block->semanticAnalysis(prog, scope);
    string block_type = block->getType(prog, scope);
    exit_scope(scope);

    if (! inherits_from(prog, block_type, return_type)){
        semanticError("return type of function is " + return_type + " but received " + block_type);
        prog.nb_errors++;
    }

}

void Method::enter_scope(SymbolTable& scope){
    formal.enter_scope(scope);
}

void Method::exit_scope(SymbolTable& scope){
    formal.exit_scope(scope);
}

void Method::pre_codegen(VSOPProgram& prog, CodeGenerator& coder){

    llvm::Type* _return_type = coder.to_type(return_type);

    // Vector of the types of the arguments
    vector<llvm::Type*> params_type;
    // Push self type as the first argument
    params_type.push_back((llvm::Type*) parent->get_type(coder)->getPointerTo());

    for (auto& it : formal.list)
        if (it->type != "unit")
            params_type.push_back(coder.to_type(it->type));

    // Get the protoype of the function we want to define
    llvm::FunctionType* function_type = llvm::FunctionType::get(_return_type, params_type, false);

    // Declare the method with the right prototype
    llvm::Function* function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, this->get_name(), *coder.module);

    auto it = function->arg_begin();
    it->setName("self");    // As always, self is the first argument
    it++;
    
    for (auto& _it : formal.list){
        if (_it->type != "unit"){
            it->setName(_it->name);
            it++;
        }
    }

}

string Method::get_name(){
    return parent->name + "_" + name;
}

llvm::Function* Method::get_function(CodeGenerator& coder){
    return coder.module->getFunction(this->get_name());
}

llvm::FunctionType* Method::get_type(CodeGenerator& coder){
    llvm::Function* method = this->get_function(coder);

    if (method == nullptr)
        return nullptr;
    else
        return method->getFunctionType();
}

void Method::codegen(VSOPProgram& prog, CodeGenerator& coder){

    // Get the method that has been declared in pre_codegen
    llvm::Function* function = this->get_function(coder);
    llvm::BasicBlock* entry_point = llvm::BasicBlock::Create(*coder.context, "", function);
    coder.builder->SetInsertPoint(entry_point);

    auto it = function->arg_begin();
    
    // Now, we will add all the formals of the method + self to the scope
    coder.insert("self", it);
    it++;

    for (auto& _it : formal.list){

        if (_it->type != "unit"){
            coder.allocate(it->getName(), it->getType());
            coder.store(it->getName(), it);
            it++;
        }else{
            coder.insert(_it->name, nullptr);
        }
    }

    block->codegen(prog, coder);

    // Once the code for the block has been generated, we can remove the formals & self from the scope
    coder.remove("self");

    for (auto& _it : formal.list)
        coder.remove(_it->name);

    // Get the return type of the function
    llvm::Type* _return_type = function->getReturnType();

    llvm::Value* return_value = nullptr;

    if (is_unit(block->get_llvm_type()) && is_unit(_return_type)){
        // No need to cast here
        coder.builder->CreateRet(return_value);
    }else{
        // Cast the value of the block
        return_value = cast_to_target(prog, coder, block->expr_value, _return_type);
        if (return_value == nullptr){
            return_value = coder.default_val(return_type);
        }
        // Return the value
        coder.builder->CreateRet(return_value);
    }
}

// New class

New::New(){}

New::New(const string& type): type(type) {}

string New::print(){
    string text = "New(" + type + ")";

    if (_type != "")
        text += ":" + _type;
    return text;
}

string New::getType(VSOPProgram& prog, SymbolTable& scope){
    _type = type;
    return type;
}

void New::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){

    if (!_is_class(type, prog)){
        semanticError("trying to use New operator on unknown type: " + type);
        prog.nb_errors++;
    }

    getType(prog, scope);
}

llvm::Value* New::codegen_aux(VSOPProgram& prog, CodeGenerator& coder){
    // Get the "new" function
    llvm::Function* function = coder.module->getFunction(type + "__new");
    // Then call it, and returns its value
    if (function != nullptr)
        return coder.builder->CreateCall(function, {});
    else
        return nullptr;
}

// Node class

void Node::semanticError(const std::string& msg){
    std::cerr << file_name + ":" + std::to_string(line) + ":" + std::to_string(col) + ": semantic error: " + msg << std::endl;
}

// Self class

Self::Self(): Identifier("self") {}

llvm::Value* Self::codegen_aux(VSOPProgram& prog, CodeGenerator& coder){
    return coder.get_val("self");
}

// String class

String::String(){}

String::String(const string& name): name(name) {}

string String::print(){
    string text = "\"";

    for (char& c : name){

        switch(c){
            case '\"': text += char_to_hex(c); break;
            case '\\': text += char_to_hex(c); break;
            default:
                    if (c >= 32 && c <= 126)
                        text += c;
                    else
                        text += char_to_hex(c);
        }
    }

    text += "\"";
    
    if (_type != "")
        text += ":" + _type;
    return text;
}

string String::getType(VSOPProgram& prog, SymbolTable& scope){
    _type = "string";
    return "string";
}

void String::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){
    getType(prog, scope);
}

llvm::Value* String::codegen_aux(VSOPProgram& prog, CodeGenerator& coder){
    return coder.builder->CreateGlobalStringPtr(name, "str");
}
// Unit class

Unit::Unit(){}

string Unit::print(){
    string text = "()";
    if (_type != "")
        text += ":" + _type;

    return text;
}

string Unit::getType(VSOPProgram& prog, SymbolTable& scope){
    _type = "unit";
    return _type;
}

void Unit::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){
    getType(prog, scope);
}

llvm::Value* Unit::codegen_aux(VSOPProgram& prog, CodeGenerator& coder){
    return nullptr;
}

// UnOp class
UnOp::UnOp(){}

UnOp::UnOp(Value value, shared_ptr<Expr> expr): value(value), expr(expr) {}

UnOp::UnOp(Value value, Expr* expr): value(value), expr(expr) {}

string UnOp::print(){
    string text = "UnOp(";
    switch(value){
        case NOT: text += "not,";
                  break;
        case MINUS: text += "-,";
                    break;
        case ISNULL: text += "isnull,";
                    break;
    }

    text += expr->print() + ")";

    if (_type != "")
        text += ":" + _type;

    return text;
}

string UnOp::getType(VSOPProgram& prog, SymbolTable& scope){
    switch(value){
        case NOT: _type = "bool"; return "bool";
        case MINUS:_type = "int32"; return "int32";
        case ISNULL: _type = "bool"; return "bool";
    }
}

void UnOp::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){

    string expected_type;

    switch(value){
        case NOT: expected_type = "bool"; 
                  break;
        case MINUS:expected_type = "int32";
                    break;
        case ISNULL: expected_type = "Object"; 
                    break;
    }

    expr->semanticAnalysis(prog, scope);
    string expr_type = expr->getType(prog, scope);

    if(! inherits_from(prog, expr_type, expected_type)){
        semanticError("expected type " + expected_type + " but received type " + expr_type);
        prog.nb_errors++;
    }

    getType(prog, scope);
}

llvm::Value* UnOp::codegen_aux(VSOPProgram& prog, CodeGenerator& coder){
    // Generate code for the rhs
    expr->codegen(prog, coder);

    // Then, return the value of the corresponding method
    switch (value){
        case NOT: return coder.builder->CreateNot(expr->expr_value);

        case MINUS: return coder.builder->CreateNeg(expr->expr_value);

        case ISNULL: return coder.builder->CreateIsNull(expr->expr_value);

    }
}

// VSOPProgram class

VSOPProgram::VSOPProgram(){}

VSOPProgram::VSOPProgram(const VSOPList<Class>& program): program(program.list){}

string VSOPProgram::print(){
    return program.print();
}

void VSOPProgram::declaration(){

    // Declare first the Object class and insert it inside the class table
    auto arg_1 = make_shared<VSOPList<Formal>>();
    arg_1->push(new Formal("s", "string"));
    auto arg_2 = make_shared<VSOPList<Formal>>();
    arg_2->push(new Formal("b", "bool"));
    auto arg_3 = make_shared<VSOPList<Formal>>();
    arg_3->push(new Formal("i", "int32"));
    auto arg_4 = make_shared<VSOPList<Formal>>();
    auto arg_5 = make_shared<VSOPList<Formal>>();
    auto arg_6 = make_shared<VSOPList<Formal>>();

    auto methods = make_shared<VSOPList<Method>>();
    methods->push(new Method("print", "Object", *arg_1, nullptr));
    methods->push(new Method("inputInt32", "int32", *arg_6,  nullptr));
    methods->push(new Method("inputBool", "bool", *arg_5,  nullptr));
    methods->push(new Method("inputLine", "string", *arg_4, nullptr));
    methods->push(new Method("printInt32", "Object", *arg_3, nullptr));
    methods->push(new Method("printBool", "Object", *arg_2, nullptr));

    auto fields = make_shared<VSOPList<Field>>();

    auto object_class = make_shared<Class>("Object", "Object", *fields, *methods);

    for(const auto& method : methods->list)
        object_class->method_table[method->name] = method;

    class_table["Object"] = object_class;

    // Declare the other classes that are present inside the AST
    auto it = program.list.begin();
    while(it != program.list.end()){
        
        if(class_table.find((*it)->name) == class_table.end()){
            
            class_table[(*it)->name] = *it;
            (*it)->declaration(*this);
            it++;
            
        }else{
            
            semanticError("Redefinition of class " + (*it)->name);
            it = program.list.erase(it); // Just erase the redifined class just for not messing up other operations
            nb_errors++;
        }
    }

    // Check if the Main class is present

    if(class_table.find("Main") == class_table.end()){

        semanticError("class Main is undefined, it must be present in your program!");
        nb_errors++;

    }else{

        auto main = class_table["Main"];
        // Check if there is a main method
        if (main->method_table.find("main") == main->method_table.end()){

            semanticError("main method of class Main is undefined!");
            nb_errors++;

        }else{

            auto main_method = main->method_table["main"];
            
            if(main_method->formal.list.size() > 0 || main_method->return_type != "int32"){

                semanticError("main method must take no arguments and have a int32 return type");
                nb_errors++;

            }
            
        }
    }

    // Check inheritance now

    auto iter = program.list.begin();
    bool control = false;

    while(iter != program.list.end()){

        string class_name = (*iter)->name;
        auto _iter = (*iter);

        while(_iter->parent != "Object"){

            if(class_name == _iter->parent){
                // Means that we have a class which extends from itself... => Cycle detected
                semanticError("class " + (*iter)->name + " cannot extend class " + (*iter)->parent);
                nb_errors++;
                class_table.erase(class_name);
                iter = program.list.erase(iter);
                control = true;
                break;
            }

            if (class_table.find(_iter->parent) != class_table.end()){

                _iter = class_table[_iter->parent];

            }else{

                control = true;
                semanticError("class " + (*iter)->name + " cannot extend class " + (*iter)->parent);
                nb_errors++;
                class_table.erase(class_name);
                iter = program.list.erase(iter);              
                break;

            }

        }

        if (control){

            control = false;

        }else{

            (*iter)->parent_class = class_table[(*iter)->parent];
            iter++;

        }
    }

    // Now check if classes do not override some fields and methods

    for(auto& _class : program.list)
        _class->override(*this);
}

void VSOPProgram::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){

    program.semanticAnalysis(prog, scope);
}

void VSOPProgram::pre_codegen(VSOPProgram& prog, CodeGenerator& coder){

    for (auto& it : class_table)
        it.second->get_type(coder); // Forward declaration of the classes in the module

    for (auto& it : program.list)
        it->pre_codegen(prog, coder); // pre_codegen for each class in the module
}

void VSOPProgram::codegen(VSOPProgram& prog, CodeGenerator& coder){

    // codegen for all the classes
    program.codegen(prog, coder);

    // Define the prototype of the main function
    llvm::FunctionType* function_type = llvm::FunctionType::get(coder.to_type("int32"), {}, false);
    // Create the main function
    llvm::Function* function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, "main", *coder.module);

    llvm::BasicBlock* entry_point = llvm::BasicBlock::Create(*coder.context, "", function);
    coder.builder->SetInsertPoint(entry_point);

    auto _main = new New("Main");
    auto args = make_shared<VSOPList<Expr>>();

    // call to Main.main()
    coder.builder->CreateRet(Call(_main, "main", *args).codegen_aux(prog, coder));

    
}

// While class
While::While(){}

While::While(shared_ptr<Expr> cond, shared_ptr<Expr> body): cond(cond), body(body){}

While::While(Expr* cond, Expr* body): cond(cond), body(body){}

string While::print(){
    
    string text = "While(" + cond->print() + "," + body->print() + ")";
    if (_type != "")
        text += ":" + _type;
    return text;
}

string While::getType(VSOPProgram& prog, SymbolTable& scope){
    _type = "unit";
    return _type;
}

void While::semanticAnalysis(VSOPProgram& prog, SymbolTable& scope){

    cond->semanticAnalysis(prog, scope);
    string type_condition = cond->getType(prog, scope);

    if (type_condition != "bool"){
        semanticError("expected type bool for condition, but got type: " + type_condition);
        prog.nb_errors++;
    }

    body->semanticAnalysis(prog, scope);

    getType(prog, scope);
}

llvm::Value* While::codegen_aux(VSOPProgram& prog, CodeGenerator& coder){
    
    llvm::Function* function = coder.builder->GetInsertBlock()->getParent();
    
    // Create the basic blocks
    llvm::BasicBlock* cond_block = llvm::BasicBlock::Create(*coder.context, "cond", function);
    llvm::BasicBlock* body_block = llvm::BasicBlock::Create(*coder.context, "body", function);
    llvm::BasicBlock* exit_block = llvm::BasicBlock::Create(*coder.context, "exit", function);

    // Enter the condition
    coder.builder->CreateBr(cond_block);

    // Build the condition
    coder.builder->SetInsertPoint(cond_block);
    cond->codegen(prog, coder);
    coder.builder->CreateCondBr(cond->expr_value, body_block, exit_block);

    // Build the loop
    coder.builder->SetInsertPoint(body_block);
    body->codegen(prog, coder);
    coder.builder->CreateBr(cond_block);

    // Return after the loop
    coder.builder->SetInsertPoint(exit_block);

    // We return a nullptr because a loop is always of type unit
    return nullptr;
}
