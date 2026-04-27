#pragma once

#include "expression/ExprIntern.h"

#include <BitFlow/core/expression/Expression.h>

namespace BitFlow::Core::Expression {

inline Expr* MakeOpInterned(OpType op, std::vector<Expr*> inputs) {
    auto* e = new Expr{};
    e->op = op;
    e->inputs = std::move(inputs);
    return ExprIntern::Intern(e);
}

} // namespace BitFlow::Core::Expression