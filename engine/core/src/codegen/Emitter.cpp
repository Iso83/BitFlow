#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>

#include <string>
#include <sstream>

namespace BitFlow::Core::Codegen {

using namespace AST;

static std::string MakeMask(uint32_t bw) {
    if (bw == 64)
        return "~0ull";

    return "((1ull << " + std::to_string(bw) + ") - 1)";
}

static std::string EmitNode(const Expr* e, uint32_t bw) {
    using enum OpType;

    // --- leaf ---
    if (e->op == OpType::Const) {
        return std::to_string(e->constValue) + "ull";
    }

    if (e->op == OpType::Var) {
        return "v" + std::to_string(e->id.value());
    }

    // --- unary ---
    if (e->inputs.size() == 1) {
        std::string a = EmitNode(e->inputs[0], bw);

        switch (e->op) {
        case Neg: return "(-" + a + ")";
        case Not: return "(~" + a + ")";
        default: break;
        }
    }

    // --- binary / n-ary ---
    if (e->inputs.size() >= 2) {
        std::string lhs = EmitNode(e->inputs[0], bw);

        for (size_t i = 1; i < e->inputs.size(); ++i) {
            std::string rhs = EmitNode(e->inputs[i], bw);

            switch (e->op) {
            case Add: lhs = "(" + lhs + " + " + rhs + ")"; break;
            case Sub: lhs = "(" + lhs + " - " + rhs + ")"; break;
            case Mul: lhs = "(" + lhs + " * " + rhs + ")"; break;
            case Div: lhs = "(" + lhs + " / " + rhs + ")"; break;
            case Mod: lhs = "(" + lhs + " % " + rhs + ")"; break;

            case And: lhs = "(" + lhs + " & " + rhs + ")"; break;
            case Or:  lhs = "(" + lhs + " | " + rhs + ")"; break;
            case Xor: lhs = "(" + lhs + " ^ " + rhs + ")"; break;

            case Shl: lhs = "(" + lhs + " << " + rhs + ")"; break;
            case Shr: lhs = "(" + lhs + " >> " + rhs + ")"; break;

            case UShr:
                lhs = "((uint64_t)" + lhs + " >> " + rhs + ")";
                break;

            case RotL:
                lhs = "((" + lhs + " << " + rhs + ") | (" + lhs + " >> (" + std::to_string(bw) + " - " + rhs + ")))";
                break;

            case RotR:
                lhs = "((" + lhs + " >> " + rhs + ") | (" + lhs + " << (" + std::to_string(bw) + " - " + rhs + ")))";
                break;

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
    std::string mask = MakeMask(bitWidth);

    return "(" + expr + ") & " + mask;
}

} // namespace
