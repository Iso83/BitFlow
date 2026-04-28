#pragma once

#include <BitFlow/core/expression/Expression.h>

namespace BitFlow::Core::Eval {

bool IsFullyConstant(const Expression::ExprOld* root);

} // namespace BitFlow::Core::Eval
