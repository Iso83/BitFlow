#pragma once

#include "ExprKey.h"

#include <BitFlow/core/expression/Expression.h>

namespace BitFlow::Core::Expression {

using Key = ExprKey;

Key BuildCommutativeKey(const ExprOld* e);

} // namespace BitFlow::Core::Expression