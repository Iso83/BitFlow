#include "codegen/PerfPass.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace BitFlow::Core::Codegen {

using namespace AST;

namespace {

constexpr uint32_t kVarTag = 0x40000000u;
constexpr uint32_t kConstTag = 0x80000000u;

struct Context {
    std::unordered_map<const Expr*, uint32_t> cache;
    std::vector<Statement> statements;

    uint32_t nextStmtId = 0;
    uint32_t nextConstId = 0;

    std::unordered_map<uint32_t, std::string> valueToExpr;
    std::unordered_map<uint32_t, uint64_t> valueToConst;
    std::unordered_map<std::string, uint32_t> exprToConst;
};

bool IsStatementId(uint32_t id) {
    return (id & kVarTag) == 0u;
}

uint32_t MakeVarValueId(uint32_t varId) {
    return kVarTag | (varId & ~kVarTag);
}

uint32_t InternConstValueId(Context& ctx, const std::string& value) {
    auto it = ctx.exprToConst.find(value);
    if (it != ctx.exprToConst.end())
        return it->second;

    const uint32_t id = kConstTag | ctx.nextConstId++;
    ctx.exprToConst[value] = id;
    ctx.valueToExpr[id] = value;
    return id;
}

bool IsConstValueId(uint32_t id) {
    return (id & kConstTag) != 0u;
}

uint64_t MaskFor(uint32_t bitWidth) {
    if (bitWidth == 64U)
        return ~uint64_t{0};
    return (uint64_t{1} << bitWidth) - 1ULL;
}

uint32_t NormalizeShift(uint64_t amount, uint32_t bitWidth) {
    return static_cast<uint32_t>(amount % static_cast<uint64_t>(bitWidth));
}

std::string LeafExpr(const Expr* e, uint32_t bw) {
    if (e->op == OpType::Var)
        return "v" + std::to_string(e->id.value());

    if (bw > 64U)
        return "bf_uint(" + std::to_string(e->constValue) + "ull, " + std::to_string(bw) + ")";

    return std::to_string(e->constValue);
}

std::string FoldNary(const std::vector<std::string>& in, const char* op) {
    if (in.empty())
        return "0";
    std::string out = in.front();
    for (size_t i = 1; i < in.size(); ++i)
        out = "(" + out + " " + std::string(op) + " " + in[i] + ")";
    return out;
}

std::string BuildExpr(OpType op, const std::vector<std::string>& in) {
    switch (op) {
    case OpType::Not:
        return in.size() == 1 ? "(~" + in[0] + ")" : "0";
    case OpType::Neg:
        return in.size() == 1 ? "(-" + in[0] + ")" : "0";
    case OpType::And:
        return FoldNary(in, "&");
    case OpType::Or:
        return FoldNary(in, "|");
    case OpType::Xor:
        return FoldNary(in, "^");
    case OpType::Add:
        return FoldNary(in, "+");
    case OpType::Mul:
        return FoldNary(in, "*");
    case OpType::Sub:
        return in.size() == 2 ? "(" + in[0] + " - " + in[1] + ")" : "0";
    case OpType::Div:
        return in.size() == 2 ? "(" + in[0] + " / " + in[1] + ")" : "0";
    case OpType::Mod:
        return in.size() == 2 ? "(" + in[0] + " % " + in[1] + ")" : "0";
    case OpType::Shl:
        return in.size() == 2 ? "(" + in[0] + " << " + in[1] + ")" : "0";
    case OpType::Shr:
    case OpType::UShr:
        return in.size() == 2 ? "(" + in[0] + " >> " + in[1] + ")" : "0";
    case OpType::RotL:
        return in.size() == 2 ? "rotl(" + in[0] + ", " + in[1] + ")" : "0";
    case OpType::RotR:
        return in.size() == 2 ? "rotr(" + in[0] + ", " + in[1] + ")" : "0";
    case OpType::Ch:
        return in.size() == 3 ? "((" + in[0] + " & " + in[1] + ") ^ ((~" + in[0] + ") & " + in[2] + "))" : "0";
    case OpType::Maj:
        return in.size() == 3 ? "((" + in[0] + " & " + in[1] + ") ^ (" + in[0] + " & " + in[2] + ") ^ (" + in[1] +
                                    " & " + in[2] + "))"
                              : "0";
    case OpType::Var:
    case OpType::Const:
    default:
        return "0";
    }
}

std::string ValueExpr(uint32_t valueId, const std::unordered_map<uint32_t, std::string>& valueToExpr) {
    auto it = valueToExpr.find(valueId);
    if (it != valueToExpr.end())
        return it->second;
    if (IsStatementId(valueId))
        return "t" + std::to_string(valueId);
    return "0";
}

bool TryFoldPureOp(OpType op, const std::vector<uint64_t>& in, uint32_t bitWidth, uint64_t& outValue) {
    const uint64_t mask = MaskFor(bitWidth);

    switch (op) {
    case OpType::Not:
        if (in.size() != 1)
            return false;
        outValue = (~in[0]) & mask;
        return true;
    case OpType::Neg:
        if (in.size() != 1)
            return false;
        outValue = (~in[0] + 1ULL) & mask;
        return true;
    case OpType::And:
    case OpType::Or:
    case OpType::Xor:
    case OpType::Add:
    case OpType::Mul: {
        if (in.empty())
            return false;

        uint64_t acc = in[0] & mask;
        for (size_t i = 1; i < in.size(); ++i) {
            switch (op) {
            case OpType::And:
                acc &= in[i];
                break;
            case OpType::Or:
                acc |= in[i];
                break;
            case OpType::Xor:
                acc ^= in[i];
                break;
            case OpType::Add:
                acc += in[i];
                break;
            case OpType::Mul:
                acc *= in[i];
                break;
            default:
                return false;
            }
            acc &= mask;
        }
        outValue = acc;
        return true;
    }
    case OpType::Sub:
    case OpType::Div:
    case OpType::Mod:
    case OpType::Shl:
    case OpType::Shr:
    case OpType::UShr:
    case OpType::RotL:
    case OpType::RotR: {
        if (in.size() != 2)
            return false;

        const uint64_t a = in[0] & mask;
        const uint64_t b = in[1] & mask;

        switch (op) {
        case OpType::Sub:
            outValue = (a - b) & mask;
            return true;
        case OpType::Div:
            if (b == 0)
                return false;
            outValue = (a / b) & mask;
            return true;
        case OpType::Mod:
            if (b == 0)
                return false;
            outValue = (a % b) & mask;
            return true;
        case OpType::Shl:
            outValue = ((a << NormalizeShift(b, bitWidth)) & mask);
            return true;
        case OpType::Shr:
        case OpType::UShr:
            outValue = ((a >> NormalizeShift(b, bitWidth)) & mask);
            return true;
        case OpType::RotL: {
            const uint32_t shift = NormalizeShift(b, bitWidth);
            if (shift == 0) {
                outValue = a;
                return true;
            }
            outValue = (((a << shift) | (a >> (bitWidth - shift))) & mask);
            return true;
        }
        case OpType::RotR: {
            const uint32_t shift = NormalizeShift(b, bitWidth);
            if (shift == 0) {
                outValue = a;
                return true;
            }
            outValue = (((a >> shift) | (a << (bitWidth - shift))) & mask);
            return true;
        }
        default:
            return false;
        }
    }
    case OpType::Ch:
    case OpType::Maj:
        if (in.size() != 3)
            return false;
        if (op == OpType::Ch) {
            outValue = ((in[0] & in[1]) ^ ((~in[0]) & in[2])) & mask;
            return true;
        }
        outValue = ((in[0] & in[1]) ^ (in[0] & in[2]) ^ (in[1] & in[2])) & mask;
        return true;
    case OpType::Var:
    case OpType::Const:
    default:
        return false;
    }
}

uint32_t Visit(const Expr* e, uint32_t bw, Context& ctx) {
    auto it = ctx.cache.find(e);
    if (it != ctx.cache.end())
        return it->second;

    if (e->op == OpType::Var) {
        const uint32_t valueId = MakeVarValueId(e->id.value());
        ctx.valueToExpr[valueId] = LeafExpr(e, bw);
        ctx.cache[e] = valueId;
        return valueId;
    }

    if (e->op == OpType::Const) {
        const uint64_t maskedValue = e->constValue & MaskFor(bw);
        const uint32_t valueId = InternConstValueId(ctx, std::to_string(maskedValue));
        ctx.valueToConst[valueId] = maskedValue;
        ctx.cache[e] = valueId;
        return valueId;
    }

    std::vector<uint32_t> inputs;
    inputs.reserve(e->inputs.size());
    for (const Expr* in : e->inputs)
        inputs.push_back(Visit(in, bw, ctx));

    // Stap 20.2 — statement-level constant folding:
    // - only pure ops
    // - only when all inputs are constant
    // - always masked to bitWidth
    bool allInputsConst = !inputs.empty();
    std::vector<uint64_t> constInputs;
    constInputs.reserve(inputs.size());
    for (uint32_t in : inputs) {
        if (!IsConstValueId(in)) {
            allInputsConst = false;
            break;
        }
        auto itConst = ctx.valueToConst.find(in);
        if (itConst == ctx.valueToConst.end()) {
            allInputsConst = false;
            break;
        }
        constInputs.push_back(itConst->second);
    }

    if (allInputsConst) {
        uint64_t folded = 0;
        if (TryFoldPureOp(e->op, constInputs, bw, folded)) {
            const uint64_t masked = folded & MaskFor(bw);
            const uint32_t valueId = InternConstValueId(ctx, std::to_string(masked));
            ctx.valueToConst[valueId] = masked;
            ctx.cache[e] = valueId;
            return valueId;
        }
    }

    const uint32_t id = ctx.nextStmtId++;
    ctx.statements.push_back({id, static_cast<uint32_t>(e->op), inputs});
    ctx.cache[e] = id;
    return id;
}

} // namespace

SsaProgram BuildSSA(const Expr* root, uint32_t bitWidth) {
    SsaProgram prog{};
    if (root == nullptr || bitWidth == 0)
        return prog;

    Context ctx{};
    uint32_t resultId = Visit(root, bitWidth, ctx);
    ApplyPerfPass(ctx.statements, resultId);

    prog.result = ValueExpr(resultId, ctx.valueToExpr);
    prog.results.push_back(prog.result);

    prog.statements.reserve(ctx.statements.size());
    for (const Statement& st : ctx.statements) {
        std::vector<std::string> inputExprs;
        inputExprs.reserve(st.inputs.size());
        for (uint32_t in : st.inputs)
            inputExprs.push_back(ValueExpr(in, ctx.valueToExpr));

        prog.statements.push_back({"t" + std::to_string(st.id), BuildExpr(static_cast<OpType>(st.op), inputExprs)});
    }

    return prog;
}

} // namespace BitFlow::Core::Codegen
