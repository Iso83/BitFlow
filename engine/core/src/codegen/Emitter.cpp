#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>

#include <string>

namespace BitFlow::Core::Codegen {

using namespace AST;

static std::string MakeMask(uint32_t bw) {
    if (bw == 64)
        return "~0ull";

    return "((1ull << " + std::to_string(bw) + ") - 1ull)";
}

static std::string ApplyMask(const std::string& expr, uint32_t bw) {
    return "((" + expr + ") & " + MakeMask(bw) + ")";
}

static std::string NormalizeShift(const std::string& rhs, uint32_t bw) {
    return "((" + rhs + ") % " + std::to_string(bw) + "ull)";
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
            return ApplyMask("(~(" + a + ") + 1ull)", bw);
        case Not:
            return ApplyMask("(~(" + a + "))", bw);
        default:
            break;
        }
    }

    if (e->inputs.size() >= 2) {
        std::string lhs = EmitNode(e->inputs[0], bw);

        for (size_t i = 1; i < e->inputs.size(); ++i) {
            std::string rhs = EmitNode(e->inputs[i], bw);
            std::string sh = NormalizeShift(rhs, bw);

            switch (e->op) {
            case Add: lhs = ApplyMask("(" + lhs + " + " + rhs + ")", bw); break;
            case Sub: lhs = ApplyMask("(" + lhs + " - " + rhs + ")", bw); break;
            case Mul: lhs = ApplyMask("(" + lhs + " * " + rhs + ")", bw); break;
            case Div: lhs = ApplyMask("(" + lhs + " / " + rhs + ")", bw); break;
            case Mod: lhs = ApplyMask("(" + lhs + " % " + rhs + ")", bw); break;

            case And: lhs = ApplyMask("(" + lhs + " & " + rhs + ")", bw); break;
            case Or:  lhs = ApplyMask("(" + lhs + " | " + rhs + ")", bw); break;
            case Xor: lhs = ApplyMask("(" + lhs + " ^ " + rhs + ")", bw); break;

            case Shl:
                lhs = ApplyMask("(" + lhs + " << " + sh + ")", bw);
                break;
            case Shr:
            case UShr:
                lhs = ApplyMask("(" + lhs + " >> " + sh + ")", bw);
                break;

            case RotL: {
                lhs = ApplyMask("((" + lhs + " << " + sh + ") | (" + lhs + " >> (" + std::to_string(bw) + "ull - " + sh + ")))", bw);
                break;
            }
            case RotR: {
                lhs = ApplyMask("((" + lhs + " >> " + sh + ") | (" + lhs + " << (" + std::to_string(bw) + "ull - " + sh + ")))", bw);
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

std::string EmitCExpr(const Expr* root, uint32_t bitWidth) {
    std::string expr = EmitNode(root, bitWidth);
    return ApplyMask(expr, bitWidth);
}

} // namespace BitFlow::Core::Codegen
