#pragma once

#include <BitFlow/core/ast/Expression.h>

namespace BitFlow::Core::Expression {

inline AST::Expr* CloneExpr(const AST::Expr* e) {
    AST::Expr* n = new AST::Expr{};
    n->op = e->op;
    n->constValue = e->constValue;
    n->inputs = e->inputs;
    return n;
}

} // namespace BitFlow::Core::Expression
