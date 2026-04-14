#include "expression/ExprClone.h"
#include "rules/RuleCommon.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/rules/Rule.h>
#include <algorithm>
#include <vector>

namespace BitFlow::Core::Rules::Normalize {

using Expr = AST::Expr;

static bool Match_Order(const Expr& e) {
    if (!IsCommutative(e.op))
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (size_t i = 1; i < e.inputs.size(); ++i) {
        if (e.inputs[i - 1]->id.value() > e.inputs[i]->id.value())
            return true;
    }

    return false;
}

static Expr* Rewrite_Order(Expr& e) {
    std::vector<Expr*> sorted = e.inputs;

    std::sort(sorted.begin(), sorted.end(), [](Expr* a, Expr* b) { return a->id.value() < b->id.value(); });

    Expr* target = Expression::CloneExpr(&e);
    target->inputs = std::move(sorted);

    return target;
}

Rule Get_Order_Rule() {
    return Rule{RuleId::Normalize_Order, &Match_Order, &Rewrite_Order, Stage_Normalize};
}

} // namespace BitFlow::Core::Rules::Normalize
