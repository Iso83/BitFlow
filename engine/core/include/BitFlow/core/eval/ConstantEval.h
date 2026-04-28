#pragma once

#include <BitFlow/core/bitvector/BitVector.h>
#include <BitFlow/core/expression/Expression.h>
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
    uint64_t value = 0;
};

struct WideEvalResult {
    EvalStatus status = EvalStatus::UnsupportedOp;
    BitVector::bf_uint value = BitVector::bf_uint(0, 0);
};

// Precondition:
// - root should be fully constant (see IsFullyConstant in ConstantDetect.h)
// - if this is not true, EvalStatus::NotConstant is returned when traversal
//   encounters a non-constant node.
// Contract:
// - supports all bit widths > 0
// - for bitWidth > 64, `value` contains only the low 64 bits of the full result.
EvalResult EvaluateConstant(const Expression::ExprOld* root, uint32_t bitWidth);

// Full-width constant evaluation result. This is the explicit wide-bitwidth API.
// Contract:
// - supports all bit widths > 0
// - for bitWidth <= 64 this remains equivalent to EvaluateConstant, but with
//   result returned in bf_uint form.
WideEvalResult EvaluateConstantWide(const Expression::ExprOld* root, uint32_t bitWidth);

} // namespace BitFlow::Core::Eval
