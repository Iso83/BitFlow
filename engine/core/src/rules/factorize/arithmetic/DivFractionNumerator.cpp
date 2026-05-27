#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Factorize::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_DivFractionNumerator(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Div || e.inputs.size() != 2)
        return false;

    const Expr& lhs = (*store)[e.inputs[0]];

    return lhs.op == OpType::Div && lhs.inputs.size() == 2;
}

static ExprId Rewrite_DivFractionNumerator(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;

    const Expr& e = (*store)[id];

    BF_CORE_ASSERT(e.op == OpType::Div);
    BF_CORE_ASSERT(e.inputs.size() == 2);

    const Expr& lhs = (*store)[e.inputs[0]];

    BF_CORE_ASSERT(lhs.op == OpType::Div);
    BF_CORE_ASSERT(lhs.inputs.size() == 2);

    const Types::BitWidth bitWidth = e.bitWidth;

    const ExprId a = lhs.inputs[0];
    const ExprId b = lhs.inputs[1];
    const ExprId c = e.inputs[1];

    const ExprId newDenominator = store->create(OpType::Mul, {b, c}, bitWidth).id;

    return ctx.replace(id, store->create(OpType::Div, {a, newDenominator}, bitWidth).id);
}

Rule Get_DivFractionNumerator_Rule() {
    return Rule{DivFractionNumerator, &Match_DivFractionNumerator, &Rewrite_DivFractionNumerator, {Normalize::Order}};
}

} // namespace BitFlow::Core::Rules::Factorize::Arithmetic
