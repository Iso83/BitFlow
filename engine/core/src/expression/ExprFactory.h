#pragma once

#include "ast/ExprIntern.h"

#include <BitFlow/core/ast/Expression.h>

namespace BitFlow::Core::Expression {

inline AST::Expr* MakeOpInterned(AST::OpType op, std::vector<AST::Expr*> inputs) {
    auto* e = new AST::Expr{};
    e->op = op;
    e->inputs = std::move(inputs);
    return AST::ExprIntern::Intern(e);
}

} // namespace BitFlow::Core::Expression