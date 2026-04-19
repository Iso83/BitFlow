#include "codegen/PerfPass.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <sstream>
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
    std::unordered_map<uint32_t, uint64_t> constValues;
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

uint64_t MaskFor(uint32_t bitWidth) {
    if (bitWidth >= 64U)
        return ~uint64_t{0};
    return (uint64_t{1} << bitWidth) - 1ULL;
}

std::string FormatUnsignedLiteral(uint64_t value, uint32_t bitWidth) {
    if (bitWidth > 64U)
        return std::to_string(value) + "ull";

    std::ostringstream ss;
    ss << "0x" << std::hex << std::nouppercase << value;

    if (bitWidth <= 32U)
        ss << "u";
    else
        ss << "ull";

    return ss.str();
}

std::string LeafExpr(const Expr* e, uint32_t bw) {
    if (e->op == OpType::Var)
        return "v" + std::to_string(e->id.value());

    if (bw > 64U)
        return "bf_uint(" + FormatUnsignedLiteral(e->constValue, bw) + ", " + std::to_string(bw) + ")";

    return FormatUnsignedLiteral(e->constValue & MaskFor(bw), bw);
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
        const uint64_t value = (bw > 64U) ? static_cast<uint64_t>(e->constValue) : (e->constValue & MaskFor(bw));
        const uint32_t valueId = InternConstValueId(ctx, FormatUnsignedLiteral(value, bw));
        ctx.constValues[valueId] = value;
        ctx.cache[e] = valueId;
        return valueId;
    }

    std::vector<uint32_t> inputs;
    inputs.reserve(e->inputs.size());
    for (const Expr* in : e->inputs)
        inputs.push_back(Visit(in, bw, ctx));

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
    ApplyPerfPass(ctx.statements, resultId, ctx.constValues, bitWidth);
    for (const auto& [id, value] : ctx.constValues)
        if (!ctx.valueToExpr.count(id))
            ctx.valueToExpr[id] = FormatUnsignedLiteral((bitWidth > 64U) ? value : (value & MaskFor(bitWidth)), bitWidth);

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
