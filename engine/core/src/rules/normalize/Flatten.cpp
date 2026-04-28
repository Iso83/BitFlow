#include "expression/OpTraits.h"
#include "rules/RuleCommon.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Normalize {

using Expr = Expression::ExprOld;

static bool Match_Flatten(const Expr& e) {
    if (!Expression::IsCommutative(e.op))
        return false;

    if (e.inputs.empty())
        return false;

    for (const Expr* in : e.inputs) {
        if (IsNestedSame(e.op, in->op))
            return true;
    }

    return false;
}

static Expr* Rewrite_Flatten(Expr& e) {
    if (!Expression::IsCommutative(e.op))
        return nullptr;

    std::vector<Expr*> newInputs;
    newInputs.reserve(e.inputs.size());

    for (Expr* in : e.inputs) {
        if (IsNestedSame(e.op, in->op)) {
            for (Expr* sub : in->inputs)
                newInputs.push_back(sub);
        } else
            newInputs.push_back(in);
    }

    Expr* target = Expression::CloneExpr(&e);
    target->inputs = std::move(newInputs);
    return target;
}

Rule Get_Flatten_Rule() {
    return Rule{RuleId::Normalize_Flatten, &Match_Flatten,     &Rewrite_Flatten, Stage_Normalize, {},
                RuleFlags::None,           "Normalize_Flatten"};
}

} // namespace BitFlow::Core::Rules::Normalize
