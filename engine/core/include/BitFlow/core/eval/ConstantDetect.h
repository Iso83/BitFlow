#pragma once

#include <BitFlow/core/expression/Expression.h>

namespace BitFlow::Core::Eval {

bool IsFullyConstant(const Expression::Expr* root);

} // namespace BitFlow::Core::Eval
