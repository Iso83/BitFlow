#include "codegen/PerfPass.h"

#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace BitFlow::Core::Codegen {

using namespace Expression;

namespace {

constexpr uint32_t kVarTag = 0x40000000u;
constexpr uint32_t kConstTag = 0x80000000u;

struct Context {
    std::unordered_map<const ExprOld*, uint32_t> cache;
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

std::string LeafExpr(const ExprOld* e, uint32_t bw) {
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

uint32_t Visit(const ExprOld* e, uint32_t bw, Context& ctx) {
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
    for (const ExprOld* in : e->inputs)
        inputs.push_back(Visit(in, bw, ctx));

    const uint32_t id = ctx.nextStmtId++;
    ctx.statements.push_back({id, static_cast<uint32_t>(e->op), inputs});
    ctx.cache[e] = id;
    return id;
}

struct MultiContext {
    std::unordered_map<const ExprOld*, std::string> keyMemo;
    std::unordered_map<std::string, uint32_t> keyCount;
    std::unordered_map<std::string, std::string> keyToTemp;
    uint32_t nextTempId = 1;
};

std::string StructuralKey(const ExprOld* e, uint32_t bitWidth, MultiContext& ctx) {
    if (!e)
        return "null";

    auto it = ctx.keyMemo.find(e);
    if (it != ctx.keyMemo.end())
        return it->second;

    std::string key;
    if (e->op == OpType::Var) {
        key = "var:" + std::to_string(e->id.value());
    } else if (e->op == OpType::Const) {
        const uint64_t value =
            (bitWidth > 64U) ? static_cast<uint64_t>(e->constValue) : (e->constValue & MaskFor(bitWidth));
        key = "const:" + std::to_string(value);
    } else {
        key = "op:" + std::to_string(static_cast<uint32_t>(e->op)) + "(";
        for (size_t i = 0; i < e->inputs.size(); ++i) {
            if (i != 0)
                key += ",";
            key += StructuralKey(e->inputs[i], bitWidth, ctx);
        }
        key += ")";
    }

    ctx.keyMemo[e] = key;
    return key;
}

void CountRefs(const ExprOld* e, uint32_t bitWidth, MultiContext& ctx) {
    if (!e)
        return;
    ++ctx.keyCount[StructuralKey(e, bitWidth, ctx)];
    for (const ExprOld* in : e->inputs)
        CountRefs(in, bitWidth, ctx);
}

std::string ScheduleExpr(const ExprOld* e, uint32_t bitWidth, MultiContext& ctx, SsaProgram& prog) {
    if (!e)
        return "0";
    if (e->op == OpType::Var || e->op == OpType::Const)
        return LeafExpr(e, bitWidth);

    std::vector<std::string> inputExprs;
    inputExprs.reserve(e->inputs.size());
    for (const ExprOld* in : e->inputs)
        inputExprs.push_back(ScheduleExpr(in, bitWidth, ctx, prog));

    const std::string expr = BuildExpr(e->op, inputExprs);
    const std::string key = StructuralKey(e, bitWidth, ctx);

    if (ctx.keyCount[key] <= 1)
        return expr;

    auto it = ctx.keyToTemp.find(key);
    if (it != ctx.keyToTemp.end())
        return it->second;

    const std::string tempName = "t" + std::to_string(ctx.nextTempId++);
    ctx.keyToTemp[key] = tempName;
    prog.statements.push_back({tempName, expr});
    return tempName;
}

} // namespace

SsaProgram BuildSSA(const ExprOld* root, uint32_t bitWidth) {
    SsaProgram prog{};
    if (root == nullptr || bitWidth == 0)
        return prog;

    Context ctx{};
    uint32_t resultId = Visit(root, bitWidth, ctx);
    ApplyPerfPass(ctx.statements, resultId, ctx.constValues, bitWidth);
    for (const auto& [id, value] : ctx.constValues)
        if (!ctx.valueToExpr.count(id))
            ctx.valueToExpr[id] =
                FormatUnsignedLiteral((bitWidth > 64U) ? value : (value & MaskFor(bitWidth)), bitWidth);

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

SsaProgram BuildSSA(const std::vector<const ExprOld*>& roots, uint32_t bitWidth) {
    SsaProgram prog{};
    if (roots.empty() || bitWidth == 0U)
        return prog;

    MultiContext ctx{};
    for (const ExprOld* root : roots)
        CountRefs(root, bitWidth, ctx);

    prog.results.reserve(roots.size());
    for (const ExprOld* root : roots)
        prog.results.push_back(ScheduleExpr(root, bitWidth, ctx, prog));

    if (!prog.results.empty())
        prog.result = prog.results.front();
    return prog;
}

} // namespace BitFlow::Core::Codegen
