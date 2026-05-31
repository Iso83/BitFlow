#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>
#include <algorithm>
#include <vector>

namespace BitFlow::Core::Rules::Normalize {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_Order(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];

    if (!Expression::IsCommutative(e.op))
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (size_t i = 1; i < e.inputs.size(); ++i) {
        if (CanonicalExprLess(store, names, e.inputs[i], e.inputs[i - 1]))
            return true;
    }

    return false;
}

static ExprId Rewrite_Order(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    ExprInputs sorted = e.inputs;

    std::sort(sorted.begin(), sorted.end(), [&](ExprId a, ExprId b) { return CanonicalExprLess(store, names, a, b); });

    if (sorted == e.inputs)
        return id;

    return ctx.replace(id, store->create(e.op, std::move(sorted), e.bitWidth).id);
}

Rule Get_Order_Rule() {
    return Rule{Order, &Match_Order, &Rewrite_Order, {Flatten}};
}

} // namespace BitFlow::Core::Rules::Normalize
