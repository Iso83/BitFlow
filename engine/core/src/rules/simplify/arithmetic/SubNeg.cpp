#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_SubNeg(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Sub || e.inputs.size() != 2)
        return false;

    const Expr& rhs = (*store)[e.inputs[1]];
    return rhs.op == OpType::Neg && rhs.inputs.size() == 1;
}

static ExprId Rewrite_SubNeg(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    BF_CORE_ASSERT(e.op == OpType::Sub && e.inputs.size() == 2);

    const Expr& rhs = (*store)[e.inputs[1]];
    BF_CORE_ASSERT(rhs.op == OpType::Neg && rhs.inputs.size() == 1);

    return ctx.replace(id, store->create(OpType::Add, {e.inputs[0], rhs.inputs[0]}, e.bitWidth).id);
}

Rule Get_SubNeg_Rule() {
    return Rule{SubNeg, &Match_SubNeg, &Rewrite_SubNeg, {Normalize::Order, NegNeg, Arithmetic::AddFold}};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
