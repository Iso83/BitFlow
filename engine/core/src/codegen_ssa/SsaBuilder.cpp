#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace BitFlow::Core::Codegen {

using namespace AST;

namespace {

struct Context {
    std::unordered_map<const Expr*, std::string> cache;
    std::vector<SsaStatement> out;
    uint32_t counter = 0;
};

std::string MakeTemp(Context& ctx) {
    return "t" + std::to_string(ctx.counter++);
}

std::string LeafExpr(const Expr* e) {
    if (e->op == OpType::Var)
        return "v" + std::to_string(e->id.value());
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
        return in.size() == 3 ? "((" + in[0] + " & " + in[1] + ") ^ (" + in[0] + " & " + in[2] + ") ^ (" + in[1] + " & " + in[2] + "))" : "0";
    case OpType::Var:
    case OpType::Const:
    default:
        return "0";
    }
}

std::string Visit(const Expr* e, uint32_t bw, Context& ctx) {
    (void)bw;
    auto it = ctx.cache.find(e);
    if (it != ctx.cache.end())
        return it->second;

    // Leaf nodes stay inline (no SSA temp required by the rules).
    if (e->op == OpType::Const || e->op == OpType::Var) {
        std::string value = LeafExpr(e);
        ctx.cache[e] = value;
        return value;
    }

    // Post-order: children first.
    std::vector<std::string> inputs;
    inputs.reserve(e->inputs.size());
    for (const Expr* in : e->inputs)
        inputs.push_back(Visit(in, bw, ctx));

    // Non-leaf -> exactly one SSA temp.
    std::string expr = BuildExpr(e->op, inputs);
    std::string name = MakeTemp(ctx);

    ctx.out.push_back({name, expr});
    ctx.cache[e] = name;
    return name;
}

} // namespace

SsaProgram BuildSSA(const Expr* root, uint32_t bitWidth) {
    SsaProgram prog{};
    if (root == nullptr || bitWidth == 0 || bitWidth > 64)
        return prog;

    Context ctx{};
    prog.result = Visit(root, bitWidth, ctx);
    prog.results.push_back(prog.result);
    prog.statements = std::move(ctx.out);

    // Step 14.7 (later):
    // - C emitter over SSA statements:
    //     uint64_t t0 = ...;
    //     uint64_t t1 = ...;
    // - dead code elimination on unused temps
    // - register reuse / temp lifetime compaction
    // - true multi-output BuildSSA API
    return prog;
}

} // namespace BitFlow::Core::Codegen
