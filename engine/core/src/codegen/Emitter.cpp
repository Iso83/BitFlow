#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/codegen/Emitter.h>
#include <string>

namespace BitFlow::Core::Codegen {

using namespace AST;

namespace {

static std::string BitWidthLiteral(uint32_t bw) { return std::to_string(bw) + "ull"; }

static int GetPrecedence(OpType op) {
    switch (op) {
    case OpType::Const:
    case OpType::Var:
        return 80; // primary / leaf
    case OpType::Not:
    case OpType::Neg:
        return 70; // unary
    case OpType::Mul:
    case OpType::Div:
    case OpType::Mod:
        return 60;
    case OpType::Add:
    case OpType::Sub:
        return 50;
    case OpType::Shl:
    case OpType::Shr:
    case OpType::UShr:
    case OpType::RotL:
    case OpType::RotR:
        return 40;
    case OpType::And:
        return 30;
    case OpType::Xor:
        return 20;
    case OpType::Or:
        return 10;
    default:
        return 0;
    }
}

static bool NeedsParens(OpType parentOp, const Expr* child, bool isRightChild) {
    if (!child)
        return true;

    const int parentPrec = GetPrecedence(parentOp);
    const int childPrec = GetPrecedence(child->op);

    if (childPrec < parentPrec)
        return true;

    if (childPrec > parentPrec)
        return false;

    if (!isRightChild)
        return false;

    switch (parentOp) {
    case OpType::Add:
    case OpType::Mul:
    case OpType::And:
    case OpType::Or:
    case OpType::Xor:
        return false;
    default:
        return true;
    }
}

static std::string MakeMask(uint32_t bw) {
    if (bw == 64)
        return "~0ull";

    return "((1ull << " + std::to_string(bw) + ") - 1ull)";
}

static std::string ApplyMask(const std::string& expr, uint32_t bw) {
    return "((" + expr + ") & " + MakeMask(bw) + ")";
}

static std::string NormalizeShift(const std::string& rhs, uint32_t bw) {
    return "((" + rhs + ") % " + BitWidthLiteral(bw) + ")";
}

static std::string ComplementaryShift(const std::string& normalizedShift, uint32_t bw) {
    return "((" + BitWidthLiteral(bw) + " - " + normalizedShift + ") % " + BitWidthLiteral(bw) + ")";
}

static std::string MakeRotateExpr(const std::string& value, const std::string& shift, uint32_t bw, bool left) {
    const std::string invShift = ComplementaryShift(shift, bw);

    if (left)
        return "((" + value + " << " + shift + ") | (" + value + " >> " + invShift + "))";

    return "((" + value + " >> " + shift + ") | (" + value + " << " + invShift + "))";
}

static std::string MaybeWrapChild(const std::string& emittedChild, OpType parentOp, const Expr* child, bool isRightChild) {
    if (NeedsParens(parentOp, child, isRightChild))
        return "(" + emittedChild + ")";
    return emittedChild;
}

static std::string EmitNode(const Expr* e, uint32_t bw) {
    using enum OpType;

    if (e->op == OpType::Const)
        return ApplyMask(std::to_string(e->constValue) + "ull", bw);

    if (e->op == OpType::Var)
        return ApplyMask("v" + std::to_string(e->id.value()), bw);

    if (e->inputs.size() == 1) {
        std::string a = EmitNode(e->inputs[0], bw);

        switch (e->op) {
        case Neg:
            return ApplyMask("(~" + a + " + 1ull)", bw);
        case Not:
            return ApplyMask("(~" + a + ")", bw);
        default:
            break;
        }
    }

    if (e->inputs.size() >= 2) {
        std::string lhs = EmitNode(e->inputs[0], bw);

        for (size_t i = 1; i < e->inputs.size(); ++i) {
            std::string rhs = EmitNode(e->inputs[i], bw);
            std::string sh = NormalizeShift(rhs, bw);
            const Expr* leftExpr = (i == 1) ? e->inputs[0] : nullptr;
            const std::string lhsWrapped = MaybeWrapChild(lhs, e->op, leftExpr, false);
            const std::string rhsWrapped = MaybeWrapChild(rhs, e->op, e->inputs[i], true);

            switch (e->op) {
            case Add:
                lhs = ApplyMask(lhsWrapped + " + " + rhsWrapped, bw);
                break;
            case Sub:
                lhs = ApplyMask(lhsWrapped + " - " + rhsWrapped, bw);
                break;
            case Mul:
                lhs = ApplyMask(lhsWrapped + " * " + rhsWrapped, bw);
                break;
            case Div:
                lhs = ApplyMask(lhsWrapped + " / " + rhsWrapped, bw);
                break;
            case Mod:
                lhs = ApplyMask(lhsWrapped + " % " + rhsWrapped, bw);
                break;

            case And:
                lhs = ApplyMask(lhsWrapped + " & " + rhsWrapped, bw);
                break;
            case Or:
                lhs = ApplyMask(lhsWrapped + " | " + rhsWrapped, bw);
                break;
            case Xor:
                lhs = ApplyMask(lhsWrapped + " ^ " + rhsWrapped, bw);
                break;

            case Shl:
                lhs = ApplyMask(lhsWrapped + " << " + sh, bw);
                break;
            case Shr:
            case UShr:
                lhs = ApplyMask(lhsWrapped + " >> " + sh, bw);
                break;

            case RotL: {
                lhs = ApplyMask(MakeRotateExpr(lhs, sh, bw, true), bw);
                break;
            }
            case RotR: {
                lhs = ApplyMask(MakeRotateExpr(lhs, sh, bw, false), bw);
                break;
            }
            default:
                return "/*unsupported*/";
            }
        }

        return lhs;
    }

    return "/*invalid*/";
}

} // namespace

std::string EmitCExpr(const Expr* root, uint32_t bitWidth) {
    std::string expr = EmitNode(root, bitWidth);
    return ApplyMask(expr, bitWidth);
}

} // namespace BitFlow::Core::Codegen
