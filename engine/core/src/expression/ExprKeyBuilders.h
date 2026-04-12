#pragma once

#include "ExprKey.h"

#include <BitFlow/core/ast/Expression.h>

namespace BitFlow::Core::Expression {

using Key = ExprKey;

Key BuildCommutativeKey(const AST::Expr* e);

} // namespace BitFlow::Core::Expression