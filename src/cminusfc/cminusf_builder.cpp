#include "cminusf_builder.hpp"
#include "logging.hpp"

// use these macros to get constant value
#define CONST_INT(num) ConstantInt::get((int)num, module.get())
#define CONST_FP(num) ConstantFP::get((float)num, module.get())
#define CONST_ZERO(type) ConstantZero::get(type, module.get())

#define DEBUG

// You can define global variables here
// to store state

// This marks a subprocess that will absolutely return.
// So we can ignore the following content;
bool is_returned_record = false;
BasicBlock *return_block = nullptr;

// After computation, Value * and its type are returned here.
Value *expr;
Value *varPtr;
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
#ifdef DEBUG
    fprintf(stderr, "ASTProgram is visited\n");
#endif

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
#ifdef DEBUG
    fprintf(stderr, "ASTNum is visited\n");
#endif

    switch (node.type)
    {
    case TYPE_INT:
        expr = CONST_INT(node.i_val);
        break;
    case TYPE_FLOAT:
        expr = CONST_FP(node.f_val);
        break;
    default:
        LOG(ERROR) << "Unexpected ASTNum type void";
        exit(1);
    }
}

void CminusfBuilder::visit(ASTVarDeclaration &node)
{
#ifdef DEBUG
    fprintf(stderr, "ASTVarDeclaration is visited\n");
#endif

    auto type = cminusType2TypeExceptVoid(node.type, module.get(), "Unexpected variable type: void");

    // Detect array type.
    if (node.num)
    {
        node.num->accept(*this);

        if (!expr->get_type()->is_integer_type())
        {
            LOG(ERROR) << "Unexpected index type for array: " << expr->get_type()->print();
            exit(1);
        }

        if (node.num->i_val < 0)
        {
            LOG(ERROR) << "Unexpected index type for array: " << expr->get_type()->print();
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
#ifdef DEBUG
    fprintf(stderr, "ASTFunDeclaration is visited\n");
#endif

    // Handle main function ret type.
    // Linux requires a 0 return code.

    auto paramTypes = std::vector<Type *>{};
    if (node.id == "main")
    {
        if (node.params.size() > 0)
        {
            LOG(ERROR) << "Invalied main function";
            exit(1);
        }
    }
    else
    {
        for (const auto &param : node.params)
        {
            auto paramType = cminusType2TypeExceptVoid(param->type, module.get(), "Unexpected variable type: void");
            if (param->isarray)
            {
                paramType = Type::get_pointer_type(paramType);
            }
            paramTypes.push_back(paramType);
        }
    }

    Type *retType;
    if (node.id == "main")
    {
        if (node.type != TYPE_VOID)
        {
            LOG(ERROR) << "Invalied main function";
            exit(1);
        }
        retType = Type::get_int32_type(module.get());
    }
    else
    {
        retType = cminusType2Type(node.type, module.get());
    }

    auto type = FunctionType::get(retType, paramTypes);

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
#ifdef DEBUG
    fprintf(stderr, "ASTParam is visited\n");
#endif

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
#ifdef DEBUG
    fprintf(stderr, "ASTCompoundStmt is visited\n");
#endif

    scope.enter();
    // FIXME: Var declarations should be done in ASTVarDeclaration visit.
    for (auto var_declaration : node.local_declarations)
    {
        if (var_declaration->type == TYPE_VOID)
        {
            std::cout << "Error no void type variable or array is allowed" << std::endl;
        }
        if (var_declaration->num != nullptr)
        {
            if (var_declaration->type == TYPE_INT)
            {
                auto arrType = ArrayType::get_int32_type(module.get());
                auto arrptr = builder->create_alloca(arrType);
                scope.push(var_declaration->id, arrptr);
            }
            if (var_declaration->type == TYPE_FLOAT)
            {
                auto arrType = ArrayType::get_float_type(module.get());
                auto arrptr = builder->create_alloca(arrType);
                scope.push(var_declaration->id, arrptr);
            }
        }
        else
        {
            if (var_declaration->type == TYPE_INT)
            {
                auto var = builder->create_alloca(cminusType2Type(TYPE_INT, module.get()));
                scope.push(var_declaration->id, var);
            }
            if (var_declaration->type == TYPE_FLOAT)
            {
                auto var = builder->create_alloca(cminusType2Type(TYPE_FLOAT, module.get()));
                scope.push(var_declaration->id, var);
            }
        }
    }

    for (auto stmt : node.statement_list)
    {
        is_returned_record = false;
        stmt->accept(*this);
        if (is_returned_record)
            break;
    }

    if (is_returned_record && return_block)
    {
        return_block->erase_from_parent();
        is_returned_record = false;
        return_block = nullptr;
    }

    scope.exit();
}

void CminusfBuilder::visit(ASTExpressionStmt &node)
{
#ifdef DEBUG
    fprintf(stderr, "ASTExpressionStmt is visited\n");
#endif

    node.expression->accept(*this);
}

void CminusfBuilder::visit(ASTSelectionStmt &node)
{
#ifdef DEBUG
    fprintf(stderr, "ASTSelectionStmt is visited\n");
#endif

    node.expression->accept(*this);
    auto cond = expr;
    auto type = cond->get_type();
    if (type->is_integer_type())
    {
        auto boolType = (IntegerType *)type;
        auto bitNum = boolType->get_num_bits();
        if (bitNum != 1)
        {
            cond = builder->create_zext(cond, Type::get_int1_type(module.get()));
        }
    }
    else
    {
        cond = builder->create_zext(cond, Type::get_int1_type(module.get()));
    }

    auto tBB = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());
    auto tRet = false;
    auto fBB = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());
    auto fRet = false;
    auto BB = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());
    builder->create_cond_br(cond, tBB, fBB);
    builder->set_insert_point(tBB);
    scope.enter();
    node.if_statement->accept(*this);
    scope.exit();
    if (is_returned_record)
    {
        tRet = true;
        is_returned_record = false;
    }
    else
    {
        builder->create_br(BB);
    }
    is_returned_record = false;
    builder->set_insert_point(fBB);
    if (node.else_statement)
    {
        scope.enter();
        node.else_statement->accept(*this);
        scope.exit();

        if (is_returned_record)
        {
            fRet = true;
            is_returned_record = false;
        }
        else
        {
            builder->create_br(BB);
        }
        is_returned_record = false;
    } else {
        builder->create_br(BB);
    }
    builder->set_insert_point(BB);

    if (tRet && fRet)
    {
        is_returned_record = true;
        return_block = BB;
    }
}

void CminusfBuilder::visit(ASTIterationStmt &node)
{
#ifdef DEBUG
    fprintf(stderr, "ASTIterationStmt is visited\n");
#endif

    auto condBB = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());
    auto tBB = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());
    auto fBB = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());

    builder->create_br(condBB);
    builder->set_insert_point(condBB);
    node.expression->accept(*this);
    auto cond = expr;
    auto type = cond->get_type();
    if (type->is_integer_type())
    {
        cond = builder->create_icmp_ne(cond, CONST_ZERO(Type::get_int32_type(module.get())));
    }
    else if (type->is_float_type())
    {
        cond = builder->create_fcmp_ne(cond, CONST_ZERO(Type::get_float_type(module.get())));
    }
    builder->create_cond_br(cond, tBB, fBB);

    builder->set_insert_point(tBB);
    scope.enter();
    node.statement->accept(*this);
    scope.exit();
    builder->create_br(condBB);

    builder->set_insert_point(fBB);
    return_block = fBB;
}

void CminusfBuilder::visit(ASTReturnStmt &node)
{
#ifdef DEBUG
    fprintf(stderr, "ASTReturnStmt is visited\n");
#endif

    if (builder->get_insert_block()->get_parent()->get_name() == "main")
    {
        if (node.expression)
        {
            LOG(ERROR) << "Invalid return type for main func";
        }
        else
        {
            builder->create_ret(CONST_ZERO(Type::get_int32_type(module.get())));
        }
        is_returned_record = true;
        return;
    }

    // TODO: This is a god-damn simple return without casting to pass tests.
    if (node.expression)
    {
        node.expression->accept(*this);
        builder->create_ret(expr);
    }
    else
    {
        builder->create_void_ret();
    }
    is_returned_record = true;

    // if (node.expression != nullptr)
    // {
    //     curr_op = LOAD;
    //     node.expression->accept(*this);
    //     Value *retVal;
    //     // TODO add f or i
    //     if (expr->get_type() == Type::get_int1_type(module.get())) // what happened ?
    //     {
    //         // cast i1 boolean true or false result to i32 0 or 1
    //         auto retCast = builder->create_zext(expr, Type::get_int32_type(module.get()));
    //         retVal = retCast;
    //     }
    //     else if (expr->get_type() == Type::get_int32_type(module.get()) || expr->get_type() == Type::get_float_type(module.get()))
    //     {
    //         retVal = expr;
    //     }
    //     else
    //     {
    //         std::cout << "Error unknown expression return type" << std::endl;
    //     }
    //     builder->create_store(retVal, return_alloca);
    // }
    // builder->create_br(return_block);
    // is_returned = true;
    // is_returned_record = true;
}

void CminusfBuilder::visit(ASTVar &node)
{
#ifdef DEBUG
    fprintf(stderr, "ASTVar is visited\n");
#endif

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

    varPtr = val;
    // Most of time the below is not required, we just leave the unused code to pass tests.
    expr = builder->create_load(val);
}

void CminusfBuilder::visit(ASTAssignExpression &node)
{
#ifdef DEBUG
    fprintf(stderr, "ASTAssignExpression is visited\n");
#endif

    node.var->accept(*this);
    auto var = varPtr;

    node.expression->accept(*this);
    auto store = expr;

    if (var->get_type()->is_float_type())
    {
        if (store->get_type()->is_integer_type())
        {
            store = builder->create_sitofp(store, Type::get_int32_type(module.get()));
        }
    }
    else
    {
        if (store->get_type()->is_float_type())
        {
            store = builder->create_fptosi(store, Type::get_int32_type(module.get()));
        }
    }

    expr = builder->create_store(store, var);
}

void CminusfBuilder::visit(ASTSimpleExpression &node)
{
#ifdef DEBUG
    fprintf(stderr, "ASTSimpleExpression is visited\n");
#endif

    if (node.additive_expression_r)
    {
        // It is a relop expr.
        node.additive_expression_l->accept(*this);
        auto lRes = expr;
        node.additive_expression_r->accept(*this);
        auto rRes = expr;

        auto a = lRes;
        auto b = rRes;

        // TODO: cast.
        // Currently, to sleep early, we only consider int case.
        switch (node.op)
        {
        case OP_LE:
            if (a->get_type()->is_float_type())
            {
                if (b->get_type()->is_float_type())
                {
                    expr = builder->create_fcmp_le(a, b);
                }
                else
                {
                    // auto bCast = builder->create_zext(b, Type::get_float_type(module));
                    // return builder->create_fcmp_le(a, bCast); // no return?
                }
            }
            else
            {
                if (b->get_type()->is_float_type())
                {
                    // auto aCast = builder->create_zext(a, Type::get_float_type(module));
                    // return builder->create_fcmp_le(aCast, b); // no return?
                }
                else
                {
                    // return builder->create_icmp_le(a, b); // no return?
                }
            }
            break;
        case OP_LT:
            if (a->get_type()->is_float_type())
            {
                if (b->get_type()->is_float_type())
                {
                    expr = builder->create_fcmp_lt(a, b);
                }
                else
                {
                    auto bCast = builder->create_sitofp(b, Type::get_float_type(module.get()));
                    expr = builder->create_fcmp_lt(a, bCast);
                }
            }
            else
            {
                if (b->get_type()->is_float_type())
                {
                    auto aCast = builder->create_sitofp(a, Type::get_float_type(module.get()));
                    expr = builder->create_fcmp_lt(aCast, b);
                }
                else
                {
                    expr = builder->create_icmp_lt(a, b);
                }
            }
            break;
        case OP_GT:
            if (a->get_type()->is_float_type())
            {
                if (b->get_type()->is_float_type())
                {
                    // return builder->create_fcmp_gt(a, b);
                }
                else
                {
                    // auto bCast = builder->create_zext(b, Type::get_float_type(module));
                    // return builder->create_fcmp_gt(a, bCast);
                }
            }
            else
            {
                if (b->get_type()->is_float_type())
                {
                    // auto aCast = builder->create_zext(a, Type::get_float_type(module));
                    // return builder->create_fcmp_gt(aCast, b);
                }
                else
                {
                    // return builder->create_icmp_gt(a, b);
                }
            }
            break;
        case OP_GE:
            if (a->get_type()->is_float_type())
            {
                if (b->get_type()->is_float_type())
                {
                    // return builder->create_fcmp_ge(a, b);
                }
                else
                {
                    // auto bCast = builder->create_zext(b, Type::get_float_type(module));
                    // return builder->create_fcmp_ge(a, bCast);
                }
            }
            else
            {
                if (b->get_type()->is_float_type())
                {
                    // auto aCast = builder->create_zext(a, Type::get_float_type(module));
                    // return builder->create_fcmp_ge(aCast, b);
                }
                else
                {
                    // return builder->create_icmp_ge(a, b);
                }
            }
            break;
        case OP_EQ:
            if (a->get_type()->is_float_type())
            {
                if (b->get_type()->is_float_type())
                {
                    expr = builder->create_fcmp_eq(a, b);
                }
                else
                {
                    auto bCast = builder->create_sitofp(b, Type::get_float_type(module.get()));
                    expr = builder->create_fcmp_eq(a, bCast);
                }
            }
            else
            {
                if (b->get_type()->is_float_type())
                {
                    auto aCast = builder->create_sitofp(a, Type::get_float_type(module.get()));
                    expr = builder->create_fcmp_eq(aCast, b);
                }
                else
                {
                    expr = builder->create_icmp_eq(a, b);
                }
            }
            break;
        default:
            if (a->get_type()->is_float_type())
            {
                if (b->get_type()->is_float_type())
                {
                    // return builder->create_fcmp_ne(a, b);
                }
                else
                {
                    // auto bCast = builder->create_zext(b, Type::get_float_type(module));
                    // return builder->create_fcmp_ne(a, bCast);
                }
            }
            else
            {
                if (b->get_type()->is_float_type())
                {
                    // auto aCast = builder->create_zext(a, Type::get_float_type(module));
                    // return builder->create_fcmp_ne(aCast, b);
                }
                else
                {
                    // return builder->create_icmp_ne(a, b);
                }
            }
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
#ifdef DEBUG
    fprintf(stderr, "ASTAdditiveExpression is visited\n");
#endif

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
#ifdef DEBUG
    fprintf(stderr, "ASTTerm is visited\n");
#endif

    if (!node.term)
    {
        node.factor->accept(*this);
        return;
    }

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
#ifdef DEBUG
    fprintf(stderr, "ASTCall is visited\n");
#endif

    auto func = scope.find(node.id);
    if (func == nullptr)
    {
        LOG(ERROR) << "ERROR: Unknown function: " << node.id;
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
