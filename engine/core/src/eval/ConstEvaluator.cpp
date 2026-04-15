#include <BitFlow/core/eval/ConstEvaluator.h>

#include <vector>

namespace BitFlow::Core::Eval {
namespace {

[[nodiscard]] uint64_t MaskForWidth(uint32_t bitWidth) {
    if (bitWidth >= 64)
        return ~uint64_t{0};

    return (uint64_t{1} << bitWidth) - 1;
}

[[nodiscard]] uint64_t MaskValue(uint64_t value, uint32_t bitWidth) {
    return value & MaskForWidth(bitWidth);
}

EvalResult MakeUnsupported(AST::OpType op) {
    EvalResult out;
    out.status = EvalStatus::UnsupportedOp;
    out.unsupportedOp = op;
    return out;
}

EvalResult EvaluateInternal(const AST::Expr* expr, uint32_t bitWidth) {
    if (expr == nullptr)
        return MakeUnsupported(AST::OpType::Var);

    if (expr->op == AST::OpType::Var)
        return EvalResult{EvalStatus::NotConstant, 0, AST::OpType::Var};

    if (expr->op == AST::OpType::Const)
        return EvalResult{EvalStatus::Success, MaskValue(expr->constValue, bitWidth), AST::OpType::Var};

    std::vector<uint64_t> args;
    args.reserve(expr->inputs.size());

    for (const AST::Expr* in : expr->inputs) {
        EvalResult child = EvaluateInternal(in, bitWidth);
        if (child.status != EvalStatus::Success)
            return child;

        args.push_back(MaskValue(child.value, bitWidth));
    }

    switch (expr->op) {
    case AST::OpType::Not:
        if (args.size() != 1)
            return MakeUnsupported(expr->op);
        return EvalResult{EvalStatus::Success, MaskValue(~args[0], bitWidth), AST::OpType::Var};

    case AST::OpType::Add: {
        if (args.empty())
            return MakeUnsupported(expr->op);

        uint64_t acc = 0;
        for (uint64_t value : args)
            acc = MaskValue(acc + value, bitWidth);

        return EvalResult{EvalStatus::Success, acc, AST::OpType::Var};
    }

    case AST::OpType::Sub:
        if (args.size() != 2)
            return MakeUnsupported(expr->op);
        return EvalResult{EvalStatus::Success, MaskValue(args[0] - args[1], bitWidth), AST::OpType::Var};

    case AST::OpType::Mul: {
        if (args.empty())
            return MakeUnsupported(expr->op);

        uint64_t acc = 1;
        for (uint64_t value : args)
            acc = MaskValue(acc * value, bitWidth);

        return EvalResult{EvalStatus::Success, acc, AST::OpType::Var};
    }

    case AST::OpType::And: {
        if (args.empty())
            return MakeUnsupported(expr->op);

        uint64_t acc = args[0];
        for (size_t i = 1; i < args.size(); ++i)
            acc = MaskValue(acc & args[i], bitWidth);

        return EvalResult{EvalStatus::Success, acc, AST::OpType::Var};
    }

    case AST::OpType::Or: {
        if (args.empty())
            return MakeUnsupported(expr->op);

        uint64_t acc = args[0];
        for (size_t i = 1; i < args.size(); ++i)
            acc = MaskValue(acc | args[i], bitWidth);

        return EvalResult{EvalStatus::Success, acc, AST::OpType::Var};
    }

    case AST::OpType::Xor: {
        if (args.empty())
            return MakeUnsupported(expr->op);

        uint64_t acc = args[0];
        for (size_t i = 1; i < args.size(); ++i)
            acc = MaskValue(acc ^ args[i], bitWidth);

        return EvalResult{EvalStatus::Success, acc, AST::OpType::Var};
    }

    case AST::OpType::Shl:
    case AST::OpType::Shr:
    case AST::OpType::UShr: {
        if (args.size() != 2)
            return MakeUnsupported(expr->op);

        const uint64_t lhs = args[0];
        const uint64_t shift = args[1];

        if (shift >= bitWidth)
            return EvalResult{EvalStatus::Success, 0, AST::OpType::Var};

        uint64_t value = 0;
        if (expr->op == AST::OpType::Shl)
            value = lhs << shift;
        else
            value = lhs >> shift;

        return EvalResult{EvalStatus::Success, MaskValue(value, bitWidth), AST::OpType::Var};
    }

    case AST::OpType::RotL:
    case AST::OpType::RotR: {
        if (args.size() != 2)
            return MakeUnsupported(expr->op);

        const uint64_t lhs = args[0];
        const uint64_t shiftMod = args[1] % bitWidth;
        if (shiftMod == 0)
            return EvalResult{EvalStatus::Success, lhs, AST::OpType::Var};

        uint64_t value = 0;
        if (expr->op == AST::OpType::RotL) {
            value = MaskValue((lhs << shiftMod) | (lhs >> (bitWidth - shiftMod)), bitWidth);
        } else {
            value = MaskValue((lhs >> shiftMod) | (lhs << (bitWidth - shiftMod)), bitWidth);
        }

        return EvalResult{EvalStatus::Success, value, AST::OpType::Var};
    }

    case AST::OpType::Div:
        if (args.size() != 2)
            return MakeUnsupported(expr->op);
        if (args[1] == 0)
            return EvalResult{EvalStatus::DivisionByZero, 0, AST::OpType::Var};
        return EvalResult{EvalStatus::Success, MaskValue(args[0] / args[1], bitWidth), AST::OpType::Var};

    case AST::OpType::Mod:
        if (args.size() != 2)
            return MakeUnsupported(expr->op);
        if (args[1] == 0)
            return EvalResult{EvalStatus::ModuloByZero, 0, AST::OpType::Var};
        return EvalResult{EvalStatus::Success, MaskValue(args[0] % args[1], bitWidth), AST::OpType::Var};

    default:
        return MakeUnsupported(expr->op);
    }
}

} // namespace

EvalResult EvaluateConstExpr(const AST::Expr* expr, uint32_t bitWidth) {
    if (bitWidth < 1 || bitWidth > 64)
        return EvalResult{EvalStatus::InvalidBitWidth, 0, AST::OpType::Var};

    return EvaluateInternal(expr, bitWidth);
}

} // namespace BitFlow::Core::Eval
