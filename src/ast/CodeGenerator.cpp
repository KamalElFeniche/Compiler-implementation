#include "CodeGenerator.hpp"

bool is_int32(llvm::Type* type){

    if (type != nullptr && type->isIntegerTy(32))
        return true;
    else
        return false;
}

bool is_bool(llvm::Type* type){

    if (type != nullptr && type->isIntegerTy(1))
        return true;
    else
        return false;
}

bool is_string(llvm::Type* type){

    if (type != nullptr && type->isPointerTy() && type->getPointerElementType()->isIntegerTy(8))
        return true;
    else
        return false;
}

bool is_unit(llvm::Type* type){

    if (type == nullptr || type->isVoidTy())
        return true;
    else
        return false;
}

bool is__class(llvm::Type* type){

    if (type != nullptr && type->isPointerTy() && type->getPointerElementType()->isStructTy())
        return true;
    else
        return false;
}

std::string type_to_string(llvm::Type* type){
    if(is_int32(type))
        return "int32";
    if (is_bool(type))
        return "bool";
    if (is_string(type))
        return "string";
    if (is__class(type)){
        std::string class_name = type->getPointerElementType()->getStructName().str();
        return class_name.substr(class_name.find_first_of('.') + 1, std::string::npos);
    }

    return "unit";
}

bool is_same_as(llvm::Type* type_1, llvm::Type* type_2){

    if (is_int32(type_1))
        return is_int32(type_2);
    if (is_string(type_1))
        return is_string(type_2);
    if (is_bool(type_1))
        return is_bool(type_2);
    if (is_unit(type_1))
        return is_unit(type_2);
    if (is__class(type_1))
        return is__class(type_2) && type_1->getPointerElementType() == type_2->getPointerElementType();

    return false;
}

CodeGenerator::CodeGenerator(const std::string& name){
    context = std::make_shared<llvm::LLVMContext>();
    builder = std::make_shared<llvm::IRBuilder<>>(*context);
    module = std::make_shared<llvm::Module>(name, *context);
}

void CodeGenerator::insert(const std::string& var, llvm::Value* val){

    if (look_up(var)){

        scope.at(var).push_back(val);

    }else{

        scope.insert({var, {val}});
    }
}

bool CodeGenerator::look_up(const std::string& var){

    if (scope.find(var) == scope.end())
        return false;

    else
        return true;
}

void CodeGenerator::remove(const std::string& var){

    if (!look_up(var))
        return;

    if (scope.at(var).size() > 1){

        scope.at(var).pop_back();

    }else{

        scope.erase(var);
    }
}

llvm::Value* CodeGenerator::get_val(const std::string& var){

    if (look_up(var))
        return scope.at(var).back();

    else

        return nullptr;
}

llvm::Type* CodeGenerator::get_type(const std::string& var){

    if (llvm::Value* val = get_val(var))
        return val->getType()->getPointerElementType();

    return nullptr;
}

void CodeGenerator::allocate(const std::string& var, llvm::Type* type){

    if (is_unit(type))

        insert(var, nullptr);

    else

        insert(var, builder->CreateAlloca(type));
}

void CodeGenerator::store(const std::string& var, llvm::Value* val){

    if (llvm::Value* _val = get_val(var))
        builder->CreateStore(val, _val);
}

llvm::Value* CodeGenerator::load(const std::string &var){

    if (llvm::Value* _val = get_val(var))
        return builder->CreateLoad(_val);

    return nullptr;
}

llvm::Type* CodeGenerator::to_type(const std::string& type){

    if (type == "int32")
        return llvm::Type::getInt32Ty(*context);

    if (type == "string")
        return llvm::Type::getInt8PtrTy(*context);

    if (type == "bool")
        return llvm::Type::getInt1Ty(*context);

    if (type == "unit")
        return llvm::Type::getVoidTy(*context);

    llvm::StructType* _class = module->getTypeByName("struct." + type);

    if (_class != nullptr)
        return _class->getPointerTo();

    else
        return nullptr;
}

llvm::Value* CodeGenerator::default_val(llvm::Type* type){

    if (is_string(type))
        return builder->CreateGlobalStringPtr("", "str");

    if (is_bool(type) || is_int32(type))
        return llvm::Constant::getNullValue(type);

    if (is__class(type))
        return llvm::ConstantPointerNull::get((llvm::PointerType*) type);

    return nullptr;
}

llvm::Value* CodeGenerator::default_val(const std::string& type){

    return default_val(to_type(type));
}

std::string CodeGenerator::print(){

    std::string text;
    llvm::raw_string_ostream output(text);
    output << *module;
    return output.str();
}

void CodeGenerator::optimizer(){

    llvm::legacy::FunctionPassManager optimizer(module.get());

    optimizer.add(llvm::createInstructionCombiningPass());
    optimizer.add(llvm::createReassociatePass());
    optimizer.add(llvm::createGVNPass());
    optimizer.add(llvm::createCFGSimplificationPass());

    optimizer.doInitialization();

    for (auto it = module->begin(); it != module->end(); it++)
        optimizer.run(*it);
    
}