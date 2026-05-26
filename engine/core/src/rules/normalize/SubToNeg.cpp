#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Normalize::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_SubToNeg(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Sub || e.inputs.size() != 2)
        return false;

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];

    // Restrict to signed-constant forms: const - expr -> -(expr - const).
    if (lhs.op != OpType::Const || rhs.op == OpType::Const)
        return false;

    // Rewrite when lhs sorts before rhs,
    // so the larger/canonical expression becomes the base.
    return CanonicalExprLess(store, e.inputs[0], e.inputs[1]);
}

static ExprId Rewrite_SubToNeg(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    const Types::BitWidth bitWidth = e.bitWidth;
    BF_CORE_ASSERT(e.op == OpType::Sub && e.inputs.size() == 2);

    const ExprId swapped = store->create(OpType::Sub, {e.inputs[1], e.inputs[0]}, bitWidth).id;
    return store->create(OpType::Neg, {swapped}, bitWidth).id;
}

Rule Get_SubToNeg_Rule() {
    return Rule{SubToNeg, &Match_SubToNeg, &Rewrite_SubToNeg};
}

} // namespace BitFlow::Core::Rules::Normalize::Arithmetic
