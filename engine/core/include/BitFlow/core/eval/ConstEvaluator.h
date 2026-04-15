#pragma once

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <cstdint>

namespace BitFlow::Core::Eval {

enum class EvalStatus {
    Success,
    NotConstant,
    InvalidBitWidth,
    DivisionByZero,
    ModuloByZero,
    UnsupportedOp,
};

struct EvalResult {
    EvalStatus status = EvalStatus::NotConstant;
    uint64_t value = 0;
    AST::OpType unsupportedOp = AST::OpType::Var;

    [[nodiscard]] bool ok() const {
        return status == EvalStatus::Success;
    }
};

// Evaluates only fully constant expressions using fixed-width unsigned semantics.
// Supported bit-width is 1..64 and every intermediate/final value is masked.
// Shift semantics: logical shifts (Shl/Shr/UShr) return 0 when shiftcount >= bitWidth.
EvalResult EvaluateConstExpr(const AST::Expr* expr, uint32_t bitWidth);

} // namespace BitFlow::Core::Eval
