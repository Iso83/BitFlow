#include <BitFlow/engine/core/rules/RewriteContext.h>
#include <BitFlow/engine/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Engine::Core::Rules::Factorize::Bitwise {

using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;

#pragma region Helpers
static bool IsUnaryNot(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    return e.op == OpType::Not && e.inputs.size() == 1;
}

static bool Match_DeMorgan(const ExprStore* store, ExprId id, OpType op) {
    const Expr& e = (*store)[id];

    if (e.op != op || e.inputs.size() < 2)
        return false;

    for (auto in : e.inputs) {
        if (!IsUnaryNot(store, in))
            return false;
    }

    return true;
}

static ExprId Rewrite_DeMorgan(RewriteContext& ctx, ExprId id, OpType innerOp) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    const Types::BitWidth bitWidth = e.bitWidth;

    ExprInputs innerInputs;
    innerInputs.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        BF_CORE_ASSERT(exprIn.op == OpType::Not && exprIn.inputs.size() == 1);
        innerInputs.push_back(exprIn.inputs[0]);
    }

    const auto inner = store->create(innerOp, std::move(innerInputs), bitWidth).id;
    return ctx.replace(id, store->create(OpType::Not, {inner}, bitWidth).id);
}
#pragma endregion

#pragma region Match
static bool Match_DeMorganAnd(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    return Match_DeMorgan(store, id, OpType::Or);
}

static bool Match_DeMorganOr(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    return Match_DeMorgan(store, id, OpType::And);
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_DeMorganAnd(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    return Rewrite_DeMorgan(ctx, id, OpType::And);
}

static ExprId Rewrite_DeMorganOr(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    return Rewrite_DeMorgan(ctx, id, OpType::Or);
}
#pragma endregion

Rule Get_DeMorganAnd_Rule() {
    return Rule{DeMorganAnd, &Match_DeMorganAnd, &Rewrite_DeMorganAnd, {Normalize::Flatten}};
}

Rule Get_DeMorganOr_Rule() {
    return Rule{DeMorganOr, &Match_DeMorganOr, &Rewrite_DeMorganOr, {Normalize::Flatten}};
}

} // namespace BitFlow::Engine::Core::Rules::Factorize::Bitwise
