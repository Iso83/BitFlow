#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/codegen/Emitter.h>
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

std::string Visit(const Expr* e, uint32_t bw, Context& ctx) {
    auto it = ctx.cache.find(e);
    if (it != ctx.cache.end())
        return it->second;

    // Leaf nodes stay inline (no SSA temp required by the rules).
    if (e->op == OpType::Const || e->op == OpType::Var) {
        std::string value = EmitCExpr(e, bw);
        ctx.cache[e] = value;
        return value;
    }

    // Post-order: children first.
    for (const Expr* in : e->inputs)
        (void)Visit(in, bw, ctx);

    // Non-leaf -> exactly one SSA temp.
    std::string expr = EmitCExpr(e, bw);
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
    prog.statements = std::move(ctx.out);
    return prog;
}

} // namespace BitFlow::Core::Codegen
