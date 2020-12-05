#include "cminusf_builder.hpp"

#include "logging.hpp"

// use these macros to get constant value
#define CONST_INT(num) ConstantInt::get((int)num, module.get())
#define CONST_FP(num) ConstantFP::get((float)num, module.get())
#define CONST_ZERO(type) ConstantZero::get(type, module.get())

// You can define global variables here
// to store state
bool is_returned = false;
bool is_returned_record = false;
BasicBlock* return_block;

Type *numType;
Value *expression;
Value* return_alloca;

enum var_op
{
    LOAD,
    STORE
};

var_op curr_op;

Type *cminusType2Type(CminusType type, Module *module)
{
    switch (type)
    {
    case CminusType::TYPE_INT:
        return Type::get_int32_type(module);
    case CminusType::TYPE_FLOAT:
        return Type::get_float_type(module);
    default:
        return Type::get_void_type(module);
    }
}

Type *cminusType2TypeExceptVoid(CminusType type, Module *module, const std::string &voidErrMsg)
{
    switch (type)
    {
    case CminusType::TYPE_INT:
        return Type::get_int32_type(module);
    case CminusType::TYPE_FLOAT:
        return Type::get_float_type(module);
    default:
        LOG(ERROR) << voidErrMsg;
        exit(1);
    }
}

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

// Loop over declarations to accept.
//
// `main` func return type is checked,
// but its param type check is delayed to ASTFunDeclaration visit.
void CminusfBuilder::visit(ASTProgram &node)
{
    if (node.declarations.size() == 0)
    {
        LOG(ERROR) << "At least one declaration is required in a program";
        exit(1);
    }

    auto lastDeclaration = node.declarations.back();
    if (lastDeclaration->id != "main" || lastDeclaration->type != CminusType::TYPE_VOID)
    {
        LOG(ERROR) << "The last declaration in a program should be void main(void)";
        exit(1);
    }

    for (const auto &declaration : node.declarations)
    {
        declaration->accept(*this);
    }
}

// Gen `Type *` from int/float.
// TODO add expression
// Manipulating builder is made by upper visit.
void CminusfBuilder::visit(ASTNum &node)
{
    numType = cminusType2TypeExceptVoid(node.type, module.get(), "Unexpected number type: void");
    // expression = ConstantInt::get(context, CONST_INT(node.i_val));
}

void CminusfBuilder::visit(ASTVarDeclaration &node)
{
    auto type = cminusType2TypeExceptVoid(node.type, module.get(), "Unexpected variable type: void");

    // Detect array type.
    if (node.num)
    {
        node.num->accept(*this);

        if (!numType->is_integer_type())
        {
            LOG(ERROR) << "Unexpected index type for array: " << numType->print();
            exit(1);
        }

        if (node.num->i_val < 0)
        {
            LOG(ERROR) << "Unexpected index type for array: " << numType->print();
            exit(1);
        }

        type = Type::get_array_type(type, node.num->i_val);
    }

    auto var = builder->create_alloca(type);
    builder->create_store(CONST_ZERO(type), var);

    if (!scope.push(node.id, var))
    {
        LOG(ERROR) << "Redeclare variable: " << node.id;
        exit(1);
    }
}

void CminusfBuilder::visit(ASTFunDeclaration &node)
{
    if (node.id == "main" && node.params.size() != 0)
    {
        LOG(ERROR) << "main function should have 0 params";
        exit(1);
    }

    auto paramTypes = std::vector<Type *>{};
    for (const auto &param : node.params)
    {
        auto paramType = cminusType2TypeExceptVoid(param->type, module.get(), "Unexpected variable type: void");
        if (param->isarray)
        {
            paramType = Type::get_pointer_type(paramType);
        }
        paramTypes.push_back(paramType);
    }

    auto type = FunctionType::get(cminusType2Type(node.type, module.get()), paramTypes);
    auto func = Function::create(type, node.id, module.get());

    if (!scope.push(node.id, func))
    {
        LOG(ERROR) << "Redeclare function: " << node.id;
        exit(1);
    }
    scope.enter();

    auto parent_bb = builder->get_insert_block();
    auto bb = BasicBlock::create(module.get(), node.id, func);
    builder->set_insert_point(bb);

    for (const auto &param : node.params)
    {
        param->accept(*this);
    }
    node.compound_stmt->accept(*this);

    builder->set_insert_point(parent_bb);
    scope.exit();
}

void CminusfBuilder::visit(ASTParam &node)
{
    auto type = cminusType2TypeExceptVoid(node.type, module.get(), "Unexpected param type: void");
    if (node.isarray)
    {
        type = Type::get_pointer_type(type);
    }

    auto param = builder->create_alloca(type);
    if (!scope.push(node.id, param))
    {
        LOG(ERROR) << "Redeclare param: " << node.id;
        exit(1);
    }
}

void CminusfBuilder::visit(ASTCompoundStmt &node)
{
    scope.enter();
    for (auto var_declaration : node.local_declarations)
    {
        if (var_declaration->type == TYPE_VOID)
        {
            std::cout << "Error no void type variable or array is allowed" << std::endl;
        }
        // array declaration
        if (var_declaration->num != nullptr)
        {
            // TODO add f or i
            auto arrType = ArrayType::get_int32_type(module.get());
            auto arrptr = builder->create_alloca(arrType);
            scope.push(var_declaration->id, arrptr);
        }
        // normal variable declaration
        else
        {
            // auto var = builder->create_alloca(Type::get_int32_type(context));
            // scope.push(var_declaration->id, var);
        }
    }
    is_returned = false;
    for (auto stmt : node.statement_list)
    {
        stmt->accept(*this);
        if (is_returned)
            break;
    }
    is_returned_record = is_returned;
    is_returned = false;
    scope.exit();
}

void CminusfBuilder::visit(ASTExpressionStmt &node)
{
    node.expression->accept(*this);
}

void CminusfBuilder::visit(ASTSelectionStmt &node) {}

void CminusfBuilder::visit(ASTIterationStmt &node) {}

void CminusfBuilder::visit(ASTReturnStmt &node)
{
    if (node.expression != nullptr)
    {
        curr_op = LOAD;
        node.expression->accept(*this);
        Value *retVal;
        // TODO add f or i
        if (expression->get_type() == Type::get_int32_type(module.get()))
        {
            // cast i1 boolean true or false result to i32 0 or 1
            auto retCast = builder->create_zext(expression, Type::get_int32_type(module.get()));
            retVal = retCast;
        }
        else if (expression->get_type() == Type::get_int32_type(module.get()))
        {
            retVal = expression;
        }
        else
        {
            std::cout << "Error unknown expression return type" << std::endl;
        }
        builder->create_store(retVal, return_alloca);
    }
    builder->create_br(return_block);
    is_returned = true;
    is_returned_record = true;
}

void CminusfBuilder::visit(ASTVar &node) {}

void CminusfBuilder::visit(ASTAssignExpression &node) {}

void CminusfBuilder::visit(ASTSimpleExpression &node) {}

void CminusfBuilder::visit(ASTAdditiveExpression &node)
{
    if (node.additive_expression == nullptr)
    {
        curr_op = LOAD;
        node.term->accept(*this);
    }
    else
    {
        bool haveF = false;
        // TODO add f or i
        curr_op = LOAD;
        node.additive_expression->accept(*this);
        // numType->get_int32_type();
        Value *lhs = expression;
        curr_op = LOAD;
        // numType->get_int32_type();
        node.term->accept(*this);
        Value *rhs = expression;
        switch (node.op)
        {
        case OP_PLUS:
            if (haveF)
            {
                expression = builder->create_fadd(lhs, rhs);
            }
            else
            {
                expression = builder->create_iadd(lhs, rhs);
            }
            break;
        case OP_MINUS:

            if (haveF)
            {
                expression = builder->create_fsub(lhs, rhs);
            }
            else
            {
                expression = builder->create_isub(lhs, rhs);
            }
            break;
        }
    }
}

void CminusfBuilder::visit(ASTTerm &node) {}

void CminusfBuilder::visit(ASTCall &node)
{
    auto func = scope.find(node.id);
    if (func == nullptr)
    {
        std::cout << "ERROR: Unknown function: " << node.id << std::endl;
        exit(1);
    }
    std::vector<Value *> args;
    for (auto arg : node.args)
    {
        arg->accept(*this);
        args.push_back(expression);
    }
    expression = builder->create_call(func, args);
}
