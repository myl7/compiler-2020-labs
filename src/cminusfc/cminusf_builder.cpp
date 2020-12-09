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
BasicBlock *return_block;

Type *numType;

// After computation, Value * and its type are returned here.
Value *expr;

Value *return_alloca;

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

Value *castWhenBinOpRequire(Value *a, Value *b, Value *i(Value *, Value *), Value *f(Value *, Value *), IRBuilder *builder, Module *module)
{
    if (a->get_type()->is_float_type())
    {
        if (b->get_type()->is_float_type())
        {
            return f(a, b);
        }
        else
        {
            auto bCast = builder->create_zext(b, Type::get_float_type(module));
            return f(a, bCast);
        }
    }
    else
    {
        if (b->get_type()->is_float_type())
        {
            auto aCast = builder->create_zext(a, Type::get_float_type(module));
            return f(aCast, b);
        }
        else
        {
            return i(a, b);
        }
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
    switch (node.type)
    {
    TYPE_INT:
        expr = CONST_INT(node.i_val);
        break;
    TYPE_FLOAT:
        expr = CONST_FP(node.f_val);
        break;
    default:
        return;
    }
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
        if (expr->get_type() == Type::get_int32_type(module.get()))
        {
            // cast i1 boolean true or false result to i32 0 or 1
            auto retCast = builder->create_zext(expr, Type::get_int32_type(module.get()));
            retVal = retCast;
        }
        else if (expr->get_type() == Type::get_int32_type(module.get()))
        {
            retVal = expr;
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

void CminusfBuilder::visit(ASTVar &node)
{
    auto val = scope.find(node.id);
    if (val == nullptr)
    {
        LOG(ERROR) << "Get var failed: id=" << node.id;
        exit(1);
    }

    if (node.expression)
    {
        node.expression->accept(*this);
        auto index = expr;

        if (index->get_type()->is_float_type())
        {
            index = builder->create_zext(index, Type::get_int32_type(module.get()));
        }

        auto cmp = builder->create_icmp_lt(index, CONST_ZERO(Type::get_int32_type(module.get())));
        auto tBB = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());
        auto fBB = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());
        builder->create_cond_br(cmp, tBB, fBB);
        builder->set_insert_point(tBB);
        builder->create_call(scope.find("neg_idx_except"), {});
        builder->set_insert_point(fBB);

        Value *val = builder->create_gep(val, {CONST_ZERO(Type::get_int32_type(module.get())), index});
    }

    expr = val;
}

void CminusfBuilder::visit(ASTAssignExpression &node)
{
    node.var->accept(*this);
    auto var = expr;

    node.expression->accept(*this);
    auto store = expr;

    if (var->get_type()->is_float_type()) {
        if (store->get_type()->is_integer_type()) {
            store = builder->create_zext(store, Type::get_float_type(module.get()));
        }
    } else {
        if (store->get_type()->is_float_type()) {
            store = builder->create_zext(store, Type::get_int32_type(module.get()));
        }
    }

    expr = builder->create_store(store, var);
}

void CminusfBuilder::visit(ASTSimpleExpression &node)
{
    if (node.additive_expression_r)
    {
        // It is a relop expr.
        node.additive_expression_l->accept(*this);
        auto lRes = expr;
        node.additive_expression_r->accept(*this);
        auto rRes = expr;

        // TODO: cast.
        // Currently, to sleep early, we only consider int case.
        switch (node.op)
        {
        case OP_LE:
            expr = castWhenBinOpRequire(lRes, rRes, builder->create_icmp_le, builder->create_fcmp_le, builder, module.get());
            break;
        case OP_LT:
            expr = castWhenBinOpRequire(lRes, rRes, builder->create_icmp_lt, builder->create_fcmp_lt, builder, module.get());
            break;
        case OP_GT:
            expr = castWhenBinOpRequire(lRes, rRes, builder->create_icmp_gt, builder->create_fcmp_gt, builder, module.get());
            break;
        case OP_GE:
            expr = castWhenBinOpRequire(lRes, rRes, builder->create_icmp_ge, builder->create_fcmp_ge, builder, module.get());
            break;
        case OP_EQ:
            expr = castWhenBinOpRequire(lRes, rRes, builder->create_icmp_eq, builder->create_fcmp_eq, builder, module.get());
            break;
        default:
            expr = castWhenBinOpRequire(lRes, rRes, builder->create_icmp_ne, builder->create_fcmp_ne, builder, module.get());
            break;
        }
    }
    else
    {
        node.additive_expression_l->accept(*this);
    }
}

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
        curr_op = LOAD;
        node.additive_expression->accept(*this);
        haveF = expr->get_type()->is_float_type() ? true : haveF;
        Value *lhs = expr;
        curr_op = LOAD;
        haveF = expr->get_type()->is_float_type() ? true : haveF;
        node.term->accept(*this);
        Value *rhs = expr;
        switch (node.op)
        {
        case OP_PLUS:
            if (haveF)
            {
                expr = builder->create_fadd(lhs, rhs);
            }
            else
            {
                expr = builder->create_iadd(lhs, rhs);
            }
            break;
        case OP_MINUS:
            if (haveF)
            {
                expr = builder->create_fsub(lhs, rhs);
            }
            else
            {
                expr = builder->create_isub(lhs, rhs);
            }
            break;
        }
    }
}

// Get left result and right result which are computed and returned from `expression` and `exprType`.
// Check their types when building statements.
//
// Feel free to refactor the variable names if you do not like them @yyw.
void CminusfBuilder::visit(ASTTerm &node)
{
    node.term->accept(*this);
    auto lRes = expr;
    auto lType = expr->get_type();

    node.factor->accept(*this);
    auto rRes = expr;
    auto rType = expr->get_type();

    if (lType->is_integer_type())
    {
        if (rType->is_float_type())
        {
            auto lCast = builder->create_zext(lRes, Type::get_float_type(module.get()));
            if (node.op == OP_MUL)
            {
                expr = builder->create_fmul(lCast, rRes);
            }
            else
            {
                expr = builder->create_fdiv(lCast, rRes);
            }
        }
        else
        {
            if (node.op == OP_MUL)
            {
                expr = builder->create_imul(lRes, rRes);
            }
            else
            {
                expr = builder->create_isdiv(lRes, rRes);
            }
        }
    }
    else
    {
        if (rType->is_float_type())
        {
            if (node.op == OP_MUL)
            {
                expr = builder->create_fmul(lRes, rRes);
            }
            else
            {
                expr = builder->create_fdiv(lRes, rRes);
            }
        }
        else
        {
            auto rCast = builder->create_zext(rRes, Type::get_float_type(module.get()));
            if (node.op == OP_MUL)
            {
                expr = builder->create_fmul(lRes, rRes);
            }
            else
            {
                expr = builder->create_fdiv(lRes, rRes);
            }
        }
    }
}

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
        args.push_back(expr);
    }
    expr = builder->create_call(func, args);
}
