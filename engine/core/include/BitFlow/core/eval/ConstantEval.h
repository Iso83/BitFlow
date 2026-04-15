#pragma once

#include <BitFlow/core/ast/Expression.h>
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

EvalResult EvaluateConstant(const AST::Expr* root, uint32_t bitWidth);

} // namespace BitFlow::Core::Eval
