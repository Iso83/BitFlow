#pragma once

#include <BitFlow/core/expression/Expression.h>

namespace BitFlow::Core::Expression {

inline Expr* CloneExpr(const Expr* e) {
    Expr* n = new Expr{};
    n->op = e->op;
    n->constValue = e->constValue;
    n->inputs = e->inputs;
    return n;
}

} // namespace BitFlow::Core::Expression
