#include "expression/OpTraits.h"
#include "rules/RuleCommon.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/rules/Rule.h>
#include <algorithm>
#include <vector>

namespace BitFlow::Core::Rules::Normalize {

using Expr = Expression::Expr;

static bool Match_Order(const Expr& e) {
    if (!Expression::IsCommutative(e.op))
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (size_t i = 1; i < e.inputs.size(); ++i) {
        if (CanonicalExprLess(e.inputs[i], e.inputs[i - 1]))
            return true;
    }

    return false;
}

static Expr* Rewrite_Order(Expr& e) {
    std::vector<Expr*> sorted = e.inputs;

    std::sort(sorted.begin(), sorted.end(), [](Expr* a, Expr* b) { return CanonicalExprLess(a, b); });

    Expr* target = Expression::CloneExpr(&e);
    target->inputs = std::move(sorted);

    return target;
}

Rule Get_Order_Rule() {
    return Rule{RuleId::Normalize_Order, &Match_Order,     &Rewrite_Order, Stage_Normalize, {},
                RuleFlags::None,         "Normalize_Order"};
}

} // namespace BitFlow::Core::Rules::Normalize
