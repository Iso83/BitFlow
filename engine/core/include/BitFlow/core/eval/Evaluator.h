#pragma once

#include <BitFlow/core/bitvector/BitVector.h>
#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/helper/Attributes.h>
#include <cstdint>

namespace BitFlow::Core::Eval {

enum class EvalStatus {
    Success,
    NotConstant,
    DivisionByZero,
    ModuloByZero,
    InvalidBitWidth,
    UnsupportedOp,
};

struct EvalResult {
    EvalStatus status = EvalStatus::UnsupportedOp;
    BitVector::bf_uint value = BitVector::bf_uint(0, 0);
};

// Evaluates a constant expression using BitVector as the canonical arithmetic model.
// Contract:
// - bitWidth must be > 0
// - all operations follow bf_uint semantics
// - returns Success if the expression is fully constant
// - otherwise returns a non-success status
EvalResult EvaluateConstant(const Expression::ExprStore* store, const Expression::Expr* root, uint32_t bitWidth);

bool IsFullyConstant(const Expression::ExprStore* store, const Expression::Expr* node);

} // namespace BitFlow::Core::Eval
