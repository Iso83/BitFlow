#include <BitFlow/core/eval/Evaluator.h>
#include <cassert>

namespace BitFlow::Core::Eval {

using namespace Expression;
using namespace BitVector;

EvalResult Make(EvalStatus status, uint32_t bitWidth) {
    return EvalResult{status, bf_uint(0, bitWidth)};
}

EvalResult MakeSuccess(const bf_uint& value) {
    return EvalResult{EvalStatus::Success, value};
}

EvalResult EvaluateConstant(const ExprStore* eStore, const Expr* node, uint32_t bitWidth) {
    assert(eStore && "EvaluateConstant: eStore must not be null");
    assert(node && "EvaluateConstant: node must not be null");

    if (bitWidth == 0)
        return Make(EvalStatus::InvalidBitWidth, 0);

    if (node == nullptr)
        return Make(EvalStatus::UnsupportedOp, bitWidth);

    switch (node->op) {
    case OpType::Var:
        return Make(EvalStatus::NotConstant, bitWidth);

    case OpType::Const:
        return MakeSuccess(bf_uint(node->knownValue, bitWidth));

    case OpType::Not:
    case OpType::Neg: {
        if (node->inputs.size() != 1)
            return Make(EvalStatus::UnsupportedOp, bitWidth);

        auto a = EvaluateConstant(eStore, &eStore->get(node->inputs[0]), bitWidth);
        if (a.status != EvalStatus::Success)
            return a;

        return (node->op == OpType::Not) ? MakeSuccess(~a.value) : MakeSuccess(-a.value);
    }

    case OpType::And:
    case OpType::Or:
    case OpType::Xor:
    case OpType::Add:
    case OpType::Mul: {
        if (node->inputs.empty())
            return Make(EvalStatus::UnsupportedOp, bitWidth);

        auto first = EvaluateConstant(eStore, &eStore->get(node->inputs[0]), bitWidth);
        if (first.status != EvalStatus::Success)
            return first;

        BitVector::bf_uint acc = std::move(first.value);

        for (size_t i = 1; i < node->inputs.size(); ++i) {
            auto term = EvaluateConstant(eStore, &eStore->get(node->inputs[i]), bitWidth);
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
                return Make(EvalStatus::UnsupportedOp, bitWidth);
            }
        }

        return MakeSuccess(std::move(acc));
    }

    case OpType::Sub:
    case OpType::Div:
    case OpType::Mod: {
        if (node->inputs.size() < 2)
            return Make(EvalStatus::UnsupportedOp, bitWidth);

        auto first = EvaluateConstant(eStore, &eStore->get(node->inputs[0]), bitWidth);
        if (first.status != EvalStatus::Success)
            return first;

        BitVector::bf_uint acc = std::move(first.value);

        for (size_t i = 1; i < node->inputs.size(); ++i) {
            auto term = EvaluateConstant(eStore, &eStore->get(node->inputs[i]), bitWidth);
            if (term.status != EvalStatus::Success)
                return term;

            switch (node->op) {
            case OpType::Sub:
                acc -= term.value;
                break;

            case OpType::Div:
                if (term.value.IsZero())
                    return Make(EvalStatus::DivisionByZero, bitWidth);
                acc /= term.value;
                break;

            case OpType::Mod:
                if (term.value.IsZero())
                    return Make(EvalStatus::ModuloByZero, bitWidth);
                acc %= term.value;
                break;

            default:
                return Make(EvalStatus::UnsupportedOp, bitWidth);
            }
        }

        return MakeSuccess(std::move(acc));
    }

    case OpType::Shl:
    case OpType::Shr:
    case OpType::RotL:
    case OpType::RotR: {
        if (node->inputs.size() != 2)
            return Make(EvalStatus::UnsupportedOp, bitWidth);

        auto a = EvaluateConstant(eStore, &eStore->get(node->inputs[0]), bitWidth);
        if (a.status != EvalStatus::Success)
            return a;

        auto b = EvaluateConstant(eStore, &eStore->get(node->inputs[1]), bitWidth);
        if (b.status != EvalStatus::Success)
            return b;

        switch (node->op) {
        case OpType::Shl:
            return MakeSuccess(a.value << b.value);

        case OpType::Shr:
            return MakeSuccess(a.value >> b.value);

        case OpType::RotL:
            return MakeSuccess(a.value.RotL(b.value.ToUint32()));

        case OpType::RotR:
            return MakeSuccess(a.value.RotR(b.value.ToUint32()));

        default:
            return Make(EvalStatus::UnsupportedOp, bitWidth);
        }
    }

    case OpType::Ch:
    case OpType::Maj: {
        if (node->inputs.size() != 3)
            return Make(EvalStatus::UnsupportedOp, bitWidth);

        auto a = EvaluateConstant(eStore, &eStore->get(node->inputs[0]), bitWidth);
        if (a.status != EvalStatus::Success)
            return a;

        auto b = EvaluateConstant(eStore, &eStore->get(node->inputs[1]), bitWidth);
        if (b.status != EvalStatus::Success)
            return b;

        auto c = EvaluateConstant(eStore, &eStore->get(node->inputs[2]), bitWidth);
        if (c.status != EvalStatus::Success)
            return c;

        if (node->op == OpType::Ch) {
            BitVector::bf_uint tmp = a.value & b.value;
            tmp ^= (~a.value) & c.value;
            return MakeSuccess(std::move(tmp));
        }

        BitVector::bf_uint tmp = a.value & b.value;
        tmp ^= (a.value & c.value);
        tmp ^= (b.value & c.value);
        return MakeSuccess(std::move(tmp));
    }

    default:
        return Make(EvalStatus::UnsupportedOp, bitWidth);
    }
}

bool IsFullyConstant(const ExprStore* eStore, const Expr* node) {
    assert(eStore && "IsFullyConstant: eStore must not be null");
    assert(node && "IsFullyConstant: node must not be null");

    if (node == nullptr)
        return false;

    if (node->op == Expression::OpType::Const)
        return true;

    if (node->op == Expression::OpType::Var)
        return false;

    if (node->inputs.empty())
        return false;

    for (auto in : node->inputs) {
        if (!IsFullyConstant(eStore, &eStore->get(in)))
            return false;
    }

    return true;
}

BF_DEPRECATED("Use EvaluateConstant with const Expr*")
EvalResult EvaluateConstant(const ExprOld* node, uint32_t bitWidth) {
    if (bitWidth == 0)
        return Make(EvalStatus::InvalidBitWidth, 0);

    if (node == nullptr)
        return Make(EvalStatus::UnsupportedOp, bitWidth);

    switch (node->op) {
    case OpType::Var:
        return Make(EvalStatus::NotConstant, bitWidth);

    case OpType::Const:
        return MakeSuccess(BitVector::bf_uint(node->constValue, bitWidth));

    case OpType::Not:
    case OpType::Neg: {
        if (node->inputs.size() != 1)
            return Make(EvalStatus::UnsupportedOp, bitWidth);

        auto a = EvaluateConstant(node->inputs[0], bitWidth);
        if (a.status != EvalStatus::Success)
            return a;

        return (node->op == OpType::Not) ? MakeSuccess(~a.value) : MakeSuccess(-a.value);
    }

    case OpType::And:
    case OpType::Or:
    case OpType::Xor:
    case OpType::Add:
    case OpType::Mul: {
        if (node->inputs.empty())
            return Make(EvalStatus::UnsupportedOp, bitWidth);

        auto first = EvaluateConstant(node->inputs[0], bitWidth);
        if (first.status != EvalStatus::Success)
            return first;

        BitVector::bf_uint acc = std::move(first.value);

        for (size_t i = 1; i < node->inputs.size(); ++i) {
            auto term = EvaluateConstant(node->inputs[i], bitWidth);
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
                return Make(EvalStatus::UnsupportedOp, bitWidth);
            }
        }

        return MakeSuccess(std::move(acc));
    }

    case OpType::Sub:
    case OpType::Div:
    case OpType::Mod: {
        if (node->inputs.size() < 2)
            return Make(EvalStatus::UnsupportedOp, bitWidth);

        auto first = EvaluateConstant(node->inputs[0], bitWidth);
        if (first.status != EvalStatus::Success)
            return first;

        BitVector::bf_uint acc = std::move(first.value);

        for (size_t i = 1; i < node->inputs.size(); ++i) {
            auto term = EvaluateConstant(node->inputs[i], bitWidth);
            if (term.status != EvalStatus::Success)
                return term;

            switch (node->op) {
            case OpType::Sub:
                acc -= term.value;
                break;

            case OpType::Div:
                if (term.value.IsZero())
                    return Make(EvalStatus::DivisionByZero, bitWidth);
                acc /= term.value;
                break;

            case OpType::Mod:
                if (term.value.IsZero())
                    return Make(EvalStatus::ModuloByZero, bitWidth);
                acc %= term.value;
                break;

            default:
                return Make(EvalStatus::UnsupportedOp, bitWidth);
            }
        }

        return MakeSuccess(std::move(acc));
    }

    case OpType::Shl:
    case OpType::Shr:
    case OpType::RotL:
    case OpType::RotR: {
        if (node->inputs.size() != 2)
            return Make(EvalStatus::UnsupportedOp, bitWidth);

        auto a = EvaluateConstant(node->inputs[0], bitWidth);
        if (a.status != EvalStatus::Success)
            return a;

        auto b = EvaluateConstant(node->inputs[1], bitWidth);
        if (b.status != EvalStatus::Success)
            return b;

        switch (node->op) {
        case OpType::Shl:
            return MakeSuccess(a.value << b.value);

        case OpType::Shr:
            return MakeSuccess(a.value >> b.value);

        case OpType::RotL:
            return MakeSuccess(a.value.RotL(b.value.ToUint32()));

        case OpType::RotR:
            return MakeSuccess(a.value.RotR(b.value.ToUint32()));

        default:
            return Make(EvalStatus::UnsupportedOp, bitWidth);
        }
    }

    case OpType::Ch:
    case OpType::Maj: {
        if (node->inputs.size() != 3)
            return Make(EvalStatus::UnsupportedOp, bitWidth);

        auto a = EvaluateConstant(node->inputs[0], bitWidth);
        if (a.status != EvalStatus::Success)
            return a;

        auto b = EvaluateConstant(node->inputs[1], bitWidth);
        if (b.status != EvalStatus::Success)
            return b;

        auto c = EvaluateConstant(node->inputs[2], bitWidth);
        if (c.status != EvalStatus::Success)
            return c;

        if (node->op == OpType::Ch) {
            BitVector::bf_uint tmp = a.value & b.value;
            tmp ^= (~a.value) & c.value;
            return MakeSuccess(std::move(tmp));
        }

        BitVector::bf_uint tmp = a.value & b.value;
        tmp ^= (a.value & c.value);
        tmp ^= (b.value & c.value);
        return MakeSuccess(std::move(tmp));
    }

    default:
        return Make(EvalStatus::UnsupportedOp, bitWidth);
    }
}
} // namespace BitFlow::Core::Eval