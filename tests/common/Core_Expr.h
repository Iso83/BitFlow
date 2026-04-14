#pragma once

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/ids/ExprId.h>

namespace BitFlow::Core::Testing {

static AST::Expr* MakeVar(uint32_t id) {
    AST::Expr* e = new AST::Expr{};
    e->id = Ids::ExprId{id};
    e->op = AST::OpType::Var;
    return e;
}

static AST::Expr* MakeOp(uint32_t id, AST::OpType op, std::initializer_list<AST::Expr*> in) {
    AST::Expr* e = new AST::Expr{};
    e->id = Ids::ExprId{id};
    e->op = op;
    e->inputs = in;
    return e;
}

static AST::Expr* MakeConst(uint32_t id, uint32_t v) {
    AST::Expr* e = new AST::Expr{};
    e->id = Ids::ExprId{id};
    e->op = AST::OpType::Const;
    e->constValue = v;
    return e;
}

using Expr = AST::Expr;
using OpType = AST::OpType;
using ConstPool = Expression::ConstPool;

} // namespace BitFlow::Core::Testing
