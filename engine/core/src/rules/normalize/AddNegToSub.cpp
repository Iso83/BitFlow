#include <BitFlow/engine/core/rules/RewriteContext.h>
#include <BitFlow/engine/core/rules/Rule.h>

namespace BitFlow::Engine::Core::Rules::Normalize::Arithmetic {

using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;

static bool Match_AddNegToSub(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Add || e.inputs.size() != 2)
        return false;

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];

    return (lhs.op == OpType::Neg && lhs.inputs.size() == 1) || (rhs.op == OpType::Neg && rhs.inputs.size() == 1);
}

static ExprId Rewrite_AddNegToSub(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    ExprId positive{};
    ExprId negative{};

    for (ExprId inId : e.inputs) {
        const Expr& in = (*store)[inId];

        if (in.op == OpType::Neg)
            negative = in.inputs[0];
        else
            positive = inId;
    }

    return ctx.replace(id, store->create(OpType::Sub, {positive, negative}, e.bitWidth).id);
}

Rule Get_AddNegToSub_Rule() {
    return Rule{AddNegToSub, &Match_AddNegToSub, &Rewrite_AddNegToSub, {Normalize::Order}};
}

} // namespace BitFlow::Engine::Core::Rules::Normalize::Arithmetic
