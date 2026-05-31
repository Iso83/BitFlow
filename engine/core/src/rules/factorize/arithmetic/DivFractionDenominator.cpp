#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Factorize::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_DivFractionDenominator(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Div || e.inputs.size() != 2)
        return false;

    const Expr& rhs = (*store)[e.inputs[1]];

    return rhs.op == OpType::Div && rhs.inputs.size() == 2;
}

static ExprId Rewrite_DivFractionDenominator(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;

    const Expr& e = (*store)[id];

    BF_CORE_ASSERT(e.op == OpType::Div);
    BF_CORE_ASSERT(e.inputs.size() == 2);

    const Expr& rhs = (*store)[e.inputs[1]];

    BF_CORE_ASSERT(rhs.op == OpType::Div);
    BF_CORE_ASSERT(rhs.inputs.size() == 2);

    const Types::BitWidth bitWidth = e.bitWidth;

    const ExprId a = e.inputs[0];
    const ExprId b = rhs.inputs[0];
    const ExprId c = rhs.inputs[1];

    const ExprId newNumerator = store->create(OpType::Mul, {a, c}, bitWidth).id;

    return ctx.replace(id, store->create(OpType::Div, {newNumerator, b}, bitWidth).id);
}

Rule Get_DivFractionDenominator_Rule() {
    return Rule{
        DivFractionDenominator, &Match_DivFractionDenominator, &Rewrite_DivFractionDenominator, {Normalize::Order}};
}

} // namespace BitFlow::Core::Rules::Factorize::Arithmetic
