#pragma once

#include <BitFlow/core/ast/Expression.h>

namespace BitFlow::Core::Eval {

bool IsFullyConstant(const AST::Expr* root);

} // namespace BitFlow::Core::Eval
