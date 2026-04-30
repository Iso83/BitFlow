#pragma once

#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/helper/Attributes.h>
#include <BitFlow/core/ids/ExprId.h>

namespace BitFlow::Core::Testing {

BF_DEPRECATED("use Expression::ExprStore")
static Expression::ExprOld* MakeVar(uint32_t id) {
    Expression::ExprOld* e = new Expression::ExprOld{};
    e->id = Ids::ExprId{id};
    e->op = Expression::OpType::Var;
    return e;
}

BF_DEPRECATED("use Expression::ExprStore")
static Expression::ExprOld* MakeOp(uint32_t id, Expression::OpType op, std::initializer_list<Expression::ExprOld*> in) {
    Expression::ExprOld* e = new Expression::ExprOld{};
    e->id = Ids::ExprId{id};
    e->op = op;
    e->inputs = in;
    return e;
}

BF_DEPRECATED("use Expression::ExprStore")
static Expression::ExprOld* MakeConst(uint32_t id, uint32_t v) {
    Expression::ExprOld* e = new Expression::ExprOld{};
    e->id = Ids::ExprId{id};
    e->op = Expression::OpType::Const;
    e->constValue = v;
    return e;
}

} // namespace BitFlow::Core::Testing
