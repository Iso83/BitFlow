#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <algorithm>
#include <vector>

namespace BitFlow::Core::Rules::Normalize {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_Order(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (!Expression::IsCommutative(e.op))
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (size_t i = 1; i < e.inputs.size(); ++i) {
        if (CanonicalExprLess(store, e.inputs[i], e.inputs[i - 1]))
            return true;
    }

    return false;
}

static ExprId Rewrite_Order(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    std::vector<ExprId> sorted = e.inputs;

    std::sort(sorted.begin(), sorted.end(), [&](ExprId a, ExprId b) { return CanonicalExprLess(store, a, b); });

    if (sorted == e.inputs)
        return id;

    return store->create(e.op, std::move(sorted), e.bitWidth).id;
}

Rule Get_Order_Rule() {
    return Rule{Order, &Match_Order, &Rewrite_Order, {Flatten}};
}

} // namespace BitFlow::Core::Rules::Normalize
