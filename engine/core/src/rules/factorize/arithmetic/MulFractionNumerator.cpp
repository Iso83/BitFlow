#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Factorize::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_MulFractionNumerator(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Mul || e.inputs.size() != 2)
        return false;

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];

    const bool lhsIsFraction = lhs.op == OpType::Div && lhs.inputs.size() == 2;

    const bool rhsIsFraction = rhs.op == OpType::Div && rhs.inputs.size() == 2;

    return lhsIsFraction != rhsIsFraction;
}

static ExprId Rewrite_MulFractionNumerator(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;

    const Expr& e = (*store)[id];

    BF_CORE_ASSERT(e.op == OpType::Mul);
    BF_CORE_ASSERT(e.inputs.size() == 2);

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];

    const bool lhsIsFraction = lhs.op == OpType::Div;

    const ExprId fractionId = lhsIsFraction ? e.inputs[0] : e.inputs[1];

    const ExprId termId = lhsIsFraction ? e.inputs[1] : e.inputs[0];

    const Expr& fraction = (*store)[fractionId];

    BF_CORE_ASSERT(fraction.op == OpType::Div);
    BF_CORE_ASSERT(fraction.inputs.size() == 2);

    const Types::BitWidth bitWidth = fraction.bitWidth;

    const ExprId numeratorId = fraction.inputs[0];
    const ExprId denominatorId = fraction.inputs[1];

    const ExprId newNumerator = store->create(OpType::Mul, {numeratorId, termId}, bitWidth).id;

    return ctx.replace(id, store->create(OpType::Div, {newNumerator, denominatorId}, bitWidth).id);
}

Rule Get_MulFractionNumerator_Rule() {
    return Rule{MulFractionNumerator, &Match_MulFractionNumerator, &Rewrite_MulFractionNumerator, {Normalize::Order}};
}

} // namespace BitFlow::Core::Rules::Factorize::Arithmetic