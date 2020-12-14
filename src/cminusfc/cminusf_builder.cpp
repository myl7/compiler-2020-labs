#include "cminusf_builder.hpp"
#include "logging.hpp"

// use these macros to get constant value
#define CONST_INT(num) ConstantInt::get((int)num, module.get())
#define CONST_FP(num) ConstantFP::get((float)num, module.get())
#define CONST_ZERO(type) ConstantZero::get(type, module.get())

#define BIN_CAST(a, b, ffunc, ifunc)                                                    \
    if (a->get_type()->is_float_type())                                                 \
    {                                                                                   \
        if (b->get_type()->is_float_type())                                             \
        {                                                                               \
            expr = builder->create_##ffunc(a, b);                                       \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            auto bType = static_cast<IntegerType *>(b->get_type());                     \
            if (bType->get_num_bits() == 1)                                             \
            {                                                                           \
                b = builder->create_zext(b, Type::get_int32_type(module.get()));        \
            }                                                                           \
            auto bCast = builder->create_sitofp(b, Type::get_float_type(module.get())); \
            expr = builder->create_##ffunc(a, bCast);                                   \
        }                                                                               \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        auto aType = static_cast<IntegerType *>(a->get_type());                         \
        if (aType->get_num_bits() == 1)                                                 \
        {                                                                               \
            a = builder->create_zext(a, Type::get_int32_type(module.get()));            \
        }                                                                               \
        if (b->get_type()->is_float_type())                                             \
        {                                                                               \
            auto aCast = builder->create_sitofp(a, Type::get_float_type(module.get())); \
            expr = builder->create_##ffunc(aCast, b);                                   \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            auto bType = static_cast<IntegerType *>(b->get_type());                     \
            if (bType->get_num_bits() == 1)                                             \
            {                                                                           \
                b = builder->create_zext(b, Type::get_int32_type(module.get()));        \
            }                                                                           \
            expr = builder->create_##ifunc(a, b);                                       \
        }                                                                               \
    }

// You can define global variables here
// to store state

// This marks a subprocess that will absolutely return.
// So we can ignore the following content;
bool is_returned = false;
BasicBlock *return_block = nullptr;

// After computation, Value * and its type are returned here.
Value *expr;
Value *varPtr;
Value *return_alloca;

Argument *arg;

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
    LOG(INFO) << "ASTProgram is visited\n";

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
    LOG(INFO) << "ASTNum is visited\n";

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
    LOG(INFO) << "ASTVarDeclaration is visited\n";

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

    Value *var;
    if (!builder->get_insert_block())
    {
        var = GlobalVariable::create(node.id, module.get(), type, false, CONST_ZERO(type));
    }
    else
    {
        var = builder->create_alloca(type);
    }
    if (!scope.push(node.id, var))
    {
        LOG(ERROR) << "Redeclare variable: " << node.id;
        exit(1);
    }
}

void CminusfBuilder::visit(ASTFunDeclaration &node)
{
    LOG(INFO) << "ASTFunDeclaration is visited\n";

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

    auto args = func->get_args();
    auto i = 0;
    for (auto a = args.begin(); a != args.end(); a++, i++)
    {
        arg = *a;
        node.params[i]->accept(*this);
    }
    node.compound_stmt->accept(*this);

    builder->set_insert_point(parent_bb);
    scope.exit();
}

void CminusfBuilder::visit(ASTParam &node)
{
    LOG(INFO) << "ASTParam is visited\n";

    auto type = cminusType2TypeExceptVoid(node.type, module.get(), "Unexpected param type: void");
    if (node.isarray)
    {
        type = Type::get_pointer_type(type);
    }

    auto param = builder->create_alloca(arg->get_type());
    builder->create_store(arg, param);
    if (!scope.push(node.id, param))
    {
        LOG(ERROR) << "Redeclare param: " << node.id;
        exit(1);
    }
}

void CminusfBuilder::visit(ASTCompoundStmt &node)
{
    LOG(INFO) << "ASTCompoundStmt is visited\n";

    scope.enter();
    // FIXME: Var declarations should be done in ASTVarDeclaration visit.
    for (auto decl : node.local_declarations)
    {
        decl->accept(*this);
    }

    for (auto stmt : node.statement_list)
    {
        is_returned = false;
        stmt->accept(*this);
        if (is_returned)
            break;
    }

    if (is_returned && return_block)
    {
        return_block->erase_from_parent();
        is_returned = false;
        return_block = nullptr;
    }

    scope.exit();
}

void CminusfBuilder::visit(ASTExpressionStmt &node)
{
    LOG(INFO) << "ASTExpressionStmt is visited\n";

    node.expression->accept(*this);
    is_returned = false;
}

void CminusfBuilder::visit(ASTSelectionStmt &node)
{
    LOG(INFO) << "ASTSelectionStmt is visited\n";

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
    if (is_returned)
    {
        tRet = true;
        is_returned = false;
    }
    else
    {
        builder->create_br(BB);
    }
    is_returned = false;
    builder->set_insert_point(fBB);
    if (node.else_statement)
    {
        scope.enter();
        node.else_statement->accept(*this);
        scope.exit();

        if (is_returned)
        {
            fRet = true;
            is_returned = false;
        }
        else
        {
            builder->create_br(BB);
        }
        is_returned = false;
    }
    else
    {
        builder->create_br(BB);
    }
    builder->set_insert_point(BB);

    if (tRet && fRet)
    {
        is_returned = true;
        return_block = BB;
    }
}

void CminusfBuilder::visit(ASTIterationStmt &node)
{
    LOG(INFO) << "ASTIterationStmt is visited\n";

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
        auto itype = static_cast<IntegerType *>(type);
        if (itype->get_num_bits() == 32)
        {
            cond = builder->create_icmp_ne(cond, CONST_ZERO(Type::get_int32_type(module.get())));
        }
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
    is_returned = false;
}

void CminusfBuilder::visit(ASTReturnStmt &node)
{
    LOG(INFO) << "ASTReturnStmt is visited\n";

    is_returned = true;

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
        return;
    }

    if (node.expression)
    {
        node.expression->accept(*this);

        auto retType = builder->get_insert_block()->get_parent()->get_return_type();
        if (expr->get_type()->is_integer_type())
        {
            auto type = static_cast<IntegerType *>(expr->get_type());

            if (type->get_num_bits() == 1)
            {
                expr = builder->create_zext(expr, Type::get_int32_type(module.get()));
            }
            if (retType->is_float_type())
            {
                expr = builder->create_sitofp(expr, Type::get_float_type(module.get()));
            }
        }
        else
        {
            if (retType->is_integer_type())
            {
                expr = builder->create_fptosi(expr, Type::get_int32_type(module.get()));
            }
        }

        builder->create_ret(expr);
    }
    else
    {
        builder->create_void_ret();
    }
}

void CminusfBuilder::visit(ASTVar &node)
{
    LOG(INFO) << "ASTVar is visited\n";

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
            index = builder->create_fptosi(index, Type::get_int32_type(module.get()));
        }
        else
        {
            auto type = static_cast<IntegerType *>(index->get_type());
            if (type->get_num_bits() == 1)
            {
                index = builder->create_zext(index, Type::get_int32_type(module.get()));
            }
        }

        auto cmp = builder->create_icmp_lt(index, CONST_ZERO(Type::get_int32_type(module.get())));
        auto tBB = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());
        auto fBB = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());
        builder->create_cond_br(cmp, tBB, fBB);
        builder->set_insert_point(tBB);
        builder->create_call(scope.find("neg_idx_except"), {});
        builder->create_br(fBB);
        builder->set_insert_point(fBB);

        val = builder->create_gep(val, {CONST_ZERO(Type::get_int32_type(module.get())), index});
    }

    varPtr = val;
    // Most of time the below is not required, we just leave the unused code to pass tests.
    expr = builder->create_load(val);
}

void CminusfBuilder::visit(ASTAssignExpression &node)
{
    LOG(INFO) << "ASTAssignExpression is visited\n";

    node.var->accept(*this);
    auto var = varPtr;
    node.expression->accept(*this);
    auto store = expr;

    auto varType = var->get_type()->get_pointer_element_type();

    if (store->get_type()->is_integer_type())
    {
        auto type = static_cast<IntegerType *>(store->get_type());
        if (type->get_num_bits() == 1)
        {
            store = builder->create_zext(store, Type::get_int32_type(module.get()));
        }
        if (varType->is_float_type())
        {
            store = builder->create_sitofp(store, Type::get_float_type(module.get()));
        }
    }
    else
    {
        if (varType->is_integer_type())
        {
            store = builder->create_fptosi(store, Type::get_int32_type(module.get()));
        }
    }

    expr = builder->create_store(store, var);
}

void CminusfBuilder::visit(ASTSimpleExpression &node)
{
    LOG(INFO) << "ASTSimpleExpression is visited\n";

    if (node.additive_expression_r)
    {
        // It is a relop expr.
        node.additive_expression_l->accept(*this);
        auto lRes = expr;
        node.additive_expression_r->accept(*this);
        auto rRes = expr;

        auto a = lRes;
        auto b = rRes;

        switch (node.op)
        {
        case OP_LE:
            BIN_CAST(a, b, fcmp_le, icmp_le);
            break;
        case OP_LT:
            BIN_CAST(a, b, fcmp_lt, icmp_lt);
            break;
        case OP_GT:
            BIN_CAST(a, b, fcmp_gt, icmp_gt);
            break;
        case OP_GE:
            BIN_CAST(a, b, fcmp_ge, icmp_ge);
            break;
        case OP_EQ:
            BIN_CAST(a, b, fcmp_eq, icmp_eq);
            break;
        default:
            BIN_CAST(a, b, fcmp_ne, icmp_ne);
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
    LOG(INFO) << "ASTAdditiveExpression is visited\n";

    if (node.additive_expression == nullptr)
    {
        node.term->accept(*this);
    }
    else
    {
        node.additive_expression->accept(*this);
        auto lRes = expr;
        auto lType = lRes->get_type();
        node.term->accept(*this);
        auto rRes = expr;
        auto rType = rRes->get_type();

        if (lType->is_integer_type())
        {
            auto liType = static_cast<IntegerType *>(lType);
            if (liType->get_num_bits() == 1)
            {
                lRes = builder->create_zext(lRes, Type::get_int32_type(module.get()));
            }
        }
        if (rType->is_integer_type())
        {
            auto riType = static_cast<IntegerType *>(rType);
            if (riType->get_num_bits() == 1)
            {
                rRes = builder->create_zext(rRes, Type::get_int32_type(module.get()));
            }
        }

        if (lType->is_integer_type())
        {
            if (rType->is_float_type())
            {
                auto lCast = builder->create_sitofp(lRes, Type::get_float_type(module.get()));
                if (node.op == OP_PLUS)
                {
                    expr = builder->create_fadd(lCast, rRes);
                }
                else
                {
                    expr = builder->create_fsub(lCast, rRes);
                }
            }
            else
            {
                if (node.op == OP_PLUS)
                {
                    expr = builder->create_iadd(lRes, rRes);
                }
                else
                {
                    expr = builder->create_isub(lRes, rRes);
                }
            }
        }
        else
        {
            if (rType->is_float_type())
            {
                if (node.op == OP_PLUS)
                {
                    expr = builder->create_fadd(lRes, rRes);
                }
                else
                {
                    expr = builder->create_fsub(lRes, rRes);
                }
            }
            else
            {
                auto rCast = builder->create_sitofp(rRes, Type::get_float_type(module.get()));
                if (node.op == OP_PLUS)
                {
                    expr = builder->create_fadd(lRes, rRes);
                }
                else
                {
                    expr = builder->create_fsub(lRes, rRes);
                }
            }
        }
    }
}

// Get left result and right result which are computed and returned from `expression` and `exprType`.
// Check their types when building statements.
//
// Feel free to refactor the variable names if you do not like them @yyw.
void CminusfBuilder::visit(ASTTerm &node)
{
    LOG(INFO) << "ASTTerm is visited\n";

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
        auto liType = static_cast<IntegerType *>(lType);
        if (liType->get_num_bits() == 1)
        {
            lRes = builder->create_zext(lRes, Type::get_int32_type(module.get()));
        }
    }
    if (rType->is_integer_type())
    {
        auto riType = static_cast<IntegerType *>(rType);
        if (riType->get_num_bits() == 1)
        {
            rRes = builder->create_zext(rRes, Type::get_int32_type(module.get()));
        }
    }

    if (lType->is_integer_type())
    {
        if (rType->is_float_type())
        {
            auto lCast = builder->create_sitofp(lRes, Type::get_float_type(module.get()));
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
            auto rCast = builder->create_sitofp(rRes, Type::get_float_type(module.get()));
            if (node.op == OP_MUL)
            {
                expr = builder->create_fmul(lRes, rCast);
            }
            else
            {
                expr = builder->create_fdiv(lRes, rCast);
            }
        }
    }
}

void CminusfBuilder::visit(ASTCall &node)
{
    LOG(INFO) << "ASTCall is visited\n";

    auto func = scope.find(node.id);
    if (func == nullptr)
    {
        LOG(ERROR) << "ERROR: Unknown function: " << node.id;
        exit(1);
    }

    auto i = 0;
    if (!func->get_type()->is_function_type())
    {
        LOG(ERROR) << "Call a non-func object";
        exit(1);
    }
    auto funcType = static_cast<FunctionType *>(func->get_type());
    std::vector<Value *> args;
    for (auto argType = funcType->param_begin(); argType != funcType->param_end(); argType++, i++)
    {
        node.args[i]->accept(*this);

        if (expr->get_type()->is_integer_type())
        {
            auto type = static_cast<IntegerType *>(expr->get_type());
            if (type->get_num_bits() == 1)
            {
                expr = builder->create_zext(expr, Type::get_int32_type(module.get()));
            }
            if ((*argType)->is_float_type())
            {
                expr = builder->create_sitofp(expr, Type::get_float_type(module.get()));
            }
        }
        else
        {
            if ((*argType)->is_integer_type())
            {
                expr = builder->create_fptosi(expr, Type::get_int32_type(module.get()));
            }
        }

        args.push_back(expr);
    }

    expr = builder->create_call(func, args);
}
