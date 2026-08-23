#include "expression/ExprUtils.h"

#include <BitFlow/engine/core/rules/RewriteContext.h>
#include <BitFlow/engine/core/rules/Rule.h>

namespace BitFlow::Engine::Core::Rules::Simplify::Arithmetic {

using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;

static bool Match_NegNeg(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Neg || e.inputs.size() != 1)
        return false;

    const Expr& in = (*store)[e.inputs[0]];
    return in.op == OpType::Neg && in.inputs.size() == 1;
}

static ExprId Rewrite_NegNeg(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    BF_CORE_ASSERT(e.op == OpType::Neg && e.inputs.size() == 1);

    const Expr& e2 = (*store)[e.inputs[0]];
    BF_CORE_ASSERT(e2.op == OpType::Neg && e2.inputs.size() == 1);

    return ctx.replace(id, e2.inputs[0]);
}

Rule Get_NegNeg_Rule() {
    return Rule{NegNeg, &Match_NegNeg, &Rewrite_NegNeg};
}

} // namespace BitFlow::Engine::Core::Rules::Simplify::Arithmetic
