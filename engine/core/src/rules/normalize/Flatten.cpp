#include "expression/ExprClone.h"
#include "rules/RuleCommon.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Normalize {

using Expr = AST::Expr;

static bool Match_Flatten(const Expr& e) {
    if (!IsCommutative(e.op))
        return false;

    if (e.inputs.empty())
        return false;

    for (const Expr* in : e.inputs) {
        if (IsNestedSameOp(e, *in))
            return true;
    }

    return false;
}

static Expr* Rewrite_Flatten(Expr& e) {
    if (!IsCommutative(e.op))
        return nullptr;

    std::vector<Expr*> newInputs;
    newInputs.reserve(e.inputs.size());

    for (Expr* in : e.inputs) {
        if (IsNestedSameOp(e, *in)) {
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
    return Rule{RuleId::Normalize_Flatten, &Match_Flatten, &Rewrite_Flatten, Stage_Normalize};
}

} // namespace BitFlow::Core::Rules::Normalize
