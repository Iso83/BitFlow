#pragma once
#include <BitFlow/core/Expression.h>

namespace BitFlow::Core {

inline Expr* CloneExpr(const Expr* e) {
    Expr* n = new Expr{};
    n->op = e->op;
    n->isConst = e->isConst;
    n->constValue = e->constValue;
    n->inputs = e->inputs;
    return n;
}

} // namespace BitFlow::Core