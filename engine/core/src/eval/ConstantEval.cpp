#include <BitFlow/core/bitvector/BitVector.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <BitFlow/core/expression/OpType.h>

namespace BitFlow::Core::Eval {
namespace {

using Expr = Expression::ExprOld;
using OpType = Expression::OpType;

uint64_t MaskFor(uint32_t bitWidth) {
    if (bitWidth == 64U)
        return ~uint64_t{0};

    return (uint64_t{1} << bitWidth) - 1ULL;
}

uint32_t NormalizeShift(uint64_t amount, uint32_t bitWidth) {
    return static_cast<uint32_t>(amount % static_cast<uint64_t>(bitWidth));
}

EvalResult Make(EvalStatus status) {
    return EvalResult{status, 0};
}

EvalResult MakeSuccess(uint64_t value, uint64_t mask) {
    return EvalResult{EvalStatus::Success, value & mask};
}

EvalResult EvalExpr(const Expr* node, uint32_t bitWidth, uint64_t mask) {
    if (node == nullptr)
        return Make(EvalStatus::UnsupportedOp);

    if (node->op == OpType::Var)
        return Make(EvalStatus::NotConstant);

    if (node->op == OpType::Const)
        return MakeSuccess(node->constValue, mask);

    const auto evalChild = [&](size_t index) -> EvalResult {
        if (index >= node->inputs.size())
            return Make(EvalStatus::UnsupportedOp);

        return EvalExpr(node->inputs[index], bitWidth, mask);
    };

    switch (node->op) {
    case OpType::Not: {
        if (node->inputs.size() != 1)
            return Make(EvalStatus::UnsupportedOp);

        EvalResult a = evalChild(0);
        if (a.status != EvalStatus::Success)
            return a;

        return MakeSuccess(~a.value, mask);
    }

    case OpType::Neg: {
        if (node->inputs.size() != 1)
            return Make(EvalStatus::UnsupportedOp);

        EvalResult a = evalChild(0);
        if (a.status != EvalStatus::Success)
            return a;

        return MakeSuccess(~a.value + 1, mask);
    }

    case OpType::And:
    case OpType::Or:
    case OpType::Xor:
    case OpType::Add:
    case OpType::Mul: {
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
            case OpType::And:
                acc &= term.value;
                break;
            case OpType::Or:
                acc |= term.value;
                break;
            case OpType::Xor:
                acc ^= term.value;
                break;
            case OpType::Add:
                acc += term.value;
                break;
            case OpType::Mul:
                acc *= term.value;
                break;
            default:
                return Make(EvalStatus::UnsupportedOp);
            }

            acc &= mask;
        }

        return MakeSuccess(acc, mask);
    }

    case OpType::Sub:
    case OpType::Div:
    case OpType::Mod:
    case OpType::Shl:
    case OpType::Shr:
    case OpType::UShr:
    case OpType::RotL:
    case OpType::RotR: {
        if (node->inputs.size() != 2)
            return Make(EvalStatus::UnsupportedOp);

        EvalResult a = evalChild(0);
        if (a.status != EvalStatus::Success)
            return a;

        EvalResult b = evalChild(1);
        if (b.status != EvalStatus::Success)
            return b;

        switch (node->op) {
        case OpType::Sub:
            return MakeSuccess(a.value - b.value, mask);

        case OpType::Div:
            if (b.value == 0)
                return Make(EvalStatus::DivisionByZero);
            return MakeSuccess(a.value / b.value, mask);

        case OpType::Mod:
            if (b.value == 0)
                return Make(EvalStatus::ModuloByZero);
            return MakeSuccess(a.value % b.value, mask);

        case OpType::Shl: {
            uint32_t shift = NormalizeShift(b.value, bitWidth);
            return MakeSuccess((a.value << shift) & mask, mask);
        }

        case OpType::Shr:
        case OpType::UShr: {
            uint32_t shift = NormalizeShift(b.value, bitWidth);
            return MakeSuccess((a.value >> shift) & mask, mask);
        }

        case OpType::RotL: {
            uint32_t shift = NormalizeShift(b.value, bitWidth);
            if (shift == 0)
                return MakeSuccess(a.value, mask);

            uint64_t v = a.value & mask;
            uint64_t rotated = ((v << shift) | (v >> (bitWidth - shift))) & mask;
            return MakeSuccess(rotated, mask);
        }

        case OpType::RotR: {
            uint32_t shift = NormalizeShift(b.value, bitWidth);
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

    case OpType::Ch:
    case OpType::Maj: {
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

        if (node->op == OpType::Ch) {
            uint64_t v = (a.value & b.value) ^ ((~a.value) & c.value);
            return MakeSuccess(v, mask);
        }

        uint64_t v = (a.value & b.value) ^ (a.value & c.value) ^ (b.value & c.value);
        return MakeSuccess(v, mask);
    }

    case OpType::Var:
    case OpType::Const:
    default:
        return Make(EvalStatus::UnsupportedOp);
    }
}

WideEvalResult MakeWide(EvalStatus status, uint32_t bitWidth) {
    return WideEvalResult{status, BitVector::bf_uint(0, bitWidth)};
}

WideEvalResult MakeWideSuccess(const BitVector::bf_uint& value) {
    return WideEvalResult{EvalStatus::Success, value};
}

WideEvalResult EvalExprWideImpl(const Expr* node, uint32_t bitWidth) {
    if (node == nullptr)
        return MakeWide(EvalStatus::UnsupportedOp, bitWidth);

    if (node->op == OpType::Var)
        return MakeWide(EvalStatus::NotConstant, bitWidth);

    if (node->op == OpType::Const)
        return MakeWideSuccess(BitVector::bf_uint(node->constValue, bitWidth));

    const auto evalChild = [&](size_t index) -> WideEvalResult {
        if (index >= node->inputs.size())
            return MakeWide(EvalStatus::UnsupportedOp, bitWidth);
        return EvalExprWideImpl(node->inputs[index], bitWidth);
    };

    switch (node->op) {
    case OpType::Not: {
        if (node->inputs.size() != 1)
            return MakeWide(EvalStatus::UnsupportedOp, bitWidth);
        WideEvalResult a = evalChild(0);
        if (a.status != EvalStatus::Success)
            return a;
        return MakeWideSuccess(~a.value);
    }
    case OpType::Neg: {
        if (node->inputs.size() != 1)
            return MakeWide(EvalStatus::UnsupportedOp, bitWidth);
        WideEvalResult a = evalChild(0);
        if (a.status != EvalStatus::Success)
            return a;
        return MakeWideSuccess(-a.value);
    }
    case OpType::And:
    case OpType::Or:
    case OpType::Xor:
    case OpType::Add:
    case OpType::Mul: {
        if (node->inputs.empty())
            return MakeWide(EvalStatus::UnsupportedOp, bitWidth);
        WideEvalResult first = evalChild(0);
        if (first.status != EvalStatus::Success)
            return first;
        BitVector::bf_uint acc = first.value;
        for (size_t i = 1; i < node->inputs.size(); ++i) {
            WideEvalResult term = evalChild(i);
            if (term.status != EvalStatus::Success)
                return term;
            switch (node->op) {
            case OpType::And:
                acc = acc & term.value;
                break;
            case OpType::Or:
                acc = acc | term.value;
                break;
            case OpType::Xor:
                acc = acc ^ term.value;
                break;
            case OpType::Add:
                acc = acc + term.value;
                break;
            case OpType::Mul:
                acc = acc * term.value;
                break;
            default:
                return MakeWide(EvalStatus::UnsupportedOp, bitWidth);
            }
        }
        return MakeWideSuccess(acc);
    }
    case OpType::Sub:
    case OpType::Div:
    case OpType::Mod:
    case OpType::Shl:
    case OpType::Shr:
    case OpType::UShr:
    case OpType::RotL:
    case OpType::RotR: {
        if (node->inputs.size() != 2)
            return MakeWide(EvalStatus::UnsupportedOp, bitWidth);
        WideEvalResult a = evalChild(0);
        if (a.status != EvalStatus::Success)
            return a;
        WideEvalResult b = evalChild(1);
        if (b.status != EvalStatus::Success)
            return b;
        switch (node->op) {
        case OpType::Sub:
            return MakeWideSuccess(a.value - b.value);
        case OpType::Div:
            if (b.value.IsZero())
                return MakeWide(EvalStatus::DivisionByZero, bitWidth);
            return MakeWideSuccess(a.value / b.value);
        case OpType::Mod:
            if (b.value.IsZero())
                return MakeWide(EvalStatus::ModuloByZero, bitWidth);
            return MakeWideSuccess(a.value % b.value);
        case OpType::Shl:
            return MakeWideSuccess(a.value.Shl(b.value.ToUint32()));
        case OpType::Shr:
        case OpType::UShr:
            return MakeWideSuccess(a.value.Shr(b.value.ToUint32()));
        case OpType::RotL:
            return MakeWideSuccess(a.value.RotL(b.value.ToUint32()));
        case OpType::RotR:
            return MakeWideSuccess(a.value.RotR(b.value.ToUint32()));
        default:
            return MakeWide(EvalStatus::UnsupportedOp, bitWidth);
        }
    }
    case OpType::Ch:
    case OpType::Maj: {
        if (node->inputs.size() != 3)
            return MakeWide(EvalStatus::UnsupportedOp, bitWidth);
        WideEvalResult a = evalChild(0);
        if (a.status != EvalStatus::Success)
            return a;
        WideEvalResult b = evalChild(1);
        if (b.status != EvalStatus::Success)
            return b;
        WideEvalResult c = evalChild(2);
        if (c.status != EvalStatus::Success)
            return c;
        if (node->op == OpType::Ch)
            return MakeWideSuccess((a.value & b.value) ^ ((~a.value) & c.value));
        return MakeWideSuccess((a.value & b.value) ^ (a.value & c.value) ^ (b.value & c.value));
    }
    case OpType::Var:
    case OpType::Const:
    default:
        return MakeWide(EvalStatus::UnsupportedOp, bitWidth);
    }
}

} // namespace

EvalResult EvaluateConstant(const Expr* root, uint32_t bitWidth) {
    WideEvalResult wide = EvaluateConstantWide(root, bitWidth);
    return EvalResult{wide.status, wide.value.ToUint64()};
}

WideEvalResult EvaluateConstantWide(const Expr* root, uint32_t bitWidth) {
    if (bitWidth == 0)
        return MakeWide(EvalStatus::InvalidBitWidth, 0);

    if (bitWidth <= 64U) {
        uint64_t mask = MaskFor(bitWidth);
        EvalResult narrow = EvalExpr(root, bitWidth, mask);
        return WideEvalResult{narrow.status, BitVector::bf_uint(narrow.value, bitWidth)};
    }

    return EvalExprWideImpl(root, bitWidth);
}

} // namespace BitFlow::Core::Eval
