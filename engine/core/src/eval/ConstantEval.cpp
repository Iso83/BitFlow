#include <BitFlow/core/eval/ConstantEval.h>

#include <BitFlow/core/eval/ConstantDetect.h>

#include <BitFlow/core/ast/OpType.h>

namespace BitFlow::Core::Eval {
namespace {

uint64_t MaskFor(uint32_t bitWidth) {
    if (bitWidth >= 64)
        return ~uint64_t{0};

    return (uint64_t{1} << bitWidth) - 1;
}

EvalResult Make(EvalStatus status) {
    return EvalResult{status, 0};
}

EvalResult MakeSuccess(uint64_t value, uint64_t mask) {
    return EvalResult{EvalStatus::Success, value & mask};
}

EvalResult EvalExpr(const AST::Expr* node, uint32_t bitWidth, uint64_t mask) {
    if (node == nullptr)
        return Make(EvalStatus::UnsupportedOp);

    if (node->op == AST::OpType::Const)
        return MakeSuccess(node->constValue, mask);

    const auto evalChild = [&](size_t index) -> EvalResult {
        if (index >= node->inputs.size())
            return Make(EvalStatus::UnsupportedOp);

        return EvalExpr(node->inputs[index], bitWidth, mask);
    };

    switch (node->op) {
    case AST::OpType::Not: {
        if (node->inputs.size() != 1)
            return Make(EvalStatus::UnsupportedOp);

        EvalResult a = evalChild(0);
        if (a.status != EvalStatus::Success)
            return a;

        return MakeSuccess(~a.value, mask);
    }

    case AST::OpType::Neg: {
        if (node->inputs.size() != 1)
            return Make(EvalStatus::UnsupportedOp);

        EvalResult a = evalChild(0);
        if (a.status != EvalStatus::Success)
            return a;

        return MakeSuccess(~a.value + 1, mask);
    }

    case AST::OpType::And:
    case AST::OpType::Or:
    case AST::OpType::Xor:
    case AST::OpType::Add:
    case AST::OpType::Mul: {
        if (node->inputs.empty())
            return Make(EvalStatus::UnsupportedOp);

        EvalResult first = evalChild(0);
        if (first.status != EvalStatus::Success)
            return first;

        uint64_t acc = first.value;

        for (size_t i = 1; i < node->inputs.size(); ++i) {
            EvalResult term = evalChild(i);
            if (term.status != EvalStatus::Success)
                return term;

            switch (node->op) {
            case AST::OpType::And:
                acc &= term.value;
                break;
            case AST::OpType::Or:
                acc |= term.value;
                break;
            case AST::OpType::Xor:
                acc ^= term.value;
                break;
            case AST::OpType::Add:
                acc += term.value;
                break;
            case AST::OpType::Mul:
                acc *= term.value;
                break;
            default:
                return Make(EvalStatus::UnsupportedOp);
            }

            acc &= mask;
        }

        return MakeSuccess(acc, mask);
    }

    case AST::OpType::Sub:
    case AST::OpType::Div:
    case AST::OpType::Mod:
    case AST::OpType::Shl:
    case AST::OpType::Shr:
    case AST::OpType::UShr:
    case AST::OpType::RotL:
    case AST::OpType::RotR: {
        if (node->inputs.size() != 2)
            return Make(EvalStatus::UnsupportedOp);

        EvalResult a = evalChild(0);
        if (a.status != EvalStatus::Success)
            return a;

        EvalResult b = evalChild(1);
        if (b.status != EvalStatus::Success)
            return b;

        switch (node->op) {
        case AST::OpType::Sub:
            return MakeSuccess(a.value - b.value, mask);

        case AST::OpType::Div:
            if (b.value == 0)
                return Make(EvalStatus::DivisionByZero);
            return MakeSuccess(a.value / b.value, mask);

        case AST::OpType::Mod:
            if (b.value == 0)
                return Make(EvalStatus::ModuloByZero);
            return MakeSuccess(a.value % b.value, mask);

        case AST::OpType::Shl: {
            if (b.value >= bitWidth)
                return MakeSuccess(0, mask);

            return MakeSuccess(a.value << static_cast<uint32_t>(b.value), mask);
        }

        case AST::OpType::Shr:
        case AST::OpType::UShr: {
            if (b.value >= bitWidth)
                return MakeSuccess(0, mask);

            return MakeSuccess(a.value >> static_cast<uint32_t>(b.value), mask);
        }

        case AST::OpType::RotL: {
            uint32_t shift = static_cast<uint32_t>(b.value % bitWidth);
            if (shift == 0)
                return MakeSuccess(a.value, mask);

            uint64_t v = a.value & mask;
            uint64_t rotated = ((v << shift) | (v >> (bitWidth - shift))) & mask;
            return MakeSuccess(rotated, mask);
        }

        case AST::OpType::RotR: {
            uint32_t shift = static_cast<uint32_t>(b.value % bitWidth);
            if (shift == 0)
                return MakeSuccess(a.value, mask);

            uint64_t v = a.value & mask;
            uint64_t rotated = ((v >> shift) | (v << (bitWidth - shift))) & mask;
            return MakeSuccess(rotated, mask);
        }

        default:
            return Make(EvalStatus::UnsupportedOp);
        }
    }

    case AST::OpType::Ch:
    case AST::OpType::Maj: {
        if (node->inputs.size() != 3)
            return Make(EvalStatus::UnsupportedOp);

        EvalResult a = evalChild(0);
        if (a.status != EvalStatus::Success)
            return a;

        EvalResult b = evalChild(1);
        if (b.status != EvalStatus::Success)
            return b;

        EvalResult c = evalChild(2);
        if (c.status != EvalStatus::Success)
            return c;

        if (node->op == AST::OpType::Ch) {
            uint64_t v = (a.value & b.value) ^ ((~a.value) & c.value);
            return MakeSuccess(v, mask);
        }

        uint64_t v = (a.value & b.value) ^ (a.value & c.value) ^ (b.value & c.value);
        return MakeSuccess(v, mask);
    }

    case AST::OpType::Var:
    case AST::OpType::Const:
    default:
        return Make(EvalStatus::UnsupportedOp);
    }
}

} // namespace

EvalResult EvaluateConstant(const AST::Expr* root, uint32_t bitWidth) {
    if (bitWidth == 0 || bitWidth > 64)
        return Make(EvalStatus::InvalidBitWidth);

    if (!IsFullyConstant(root))
        return Make(EvalStatus::NotConstant);

    uint64_t mask = MaskFor(bitWidth);
    return EvalExpr(root, bitWidth, mask);
}

} // namespace BitFlow::Core::Eval
