#pragma once

#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/ids/ExprId.h>

namespace BitFlow::Core::Testing {

static Expression::Expr* MakeVar(uint32_t id) {
    Expression::Expr* e = new Expression::Expr{};
    e->id = Ids::ExprId{id};
    e->op = Expression::OpType::Var;
    return e;
}

static Expression::Expr* MakeOp(uint32_t id, Expression::OpType op, std::initializer_list<Expression::Expr*> in) {
    Expression::Expr* e = new Expression::Expr{};
    e->id = Ids::ExprId{id};
    e->op = op;
    e->inputs = in;
    return e;
}

static Expression::Expr* MakeConst(uint32_t id, uint32_t v) {
    Expression::Expr* e = new Expression::Expr{};
    e->id = Ids::ExprId{id};
    e->op = Expression::OpType::Const;
    e->constValue = v;
    return e;
}

} // namespace BitFlow::Core::Testing
