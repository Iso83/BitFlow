#include "expression/ExprClone.h"
#include "rules/RuleStage.h"
#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Normalize {

using Expr = AST::Expr;

static bool Match_Flatten(const Expr& e) {
    if (e.inputs.empty())
        return false;

    for (const Expr* in : e.inputs) {
        if (!in->isConst && !in->inputs.empty() && in->op == e.op)
            return true;
    }

    return false;
}

static Expr* Rewrite_Flatten(Expr& e) {
    std::vector<Expr*> newInputs;

    for (Expr* in : e.inputs) {
        if (!in->isConst && !in->inputs.empty() && in->op == e.op) {
            for (Expr* sub : in->inputs)
                newInputs.push_back(sub);
        } else
            newInputs.push_back(in);
    }

    Expr* target = e.frozen ? Expression::CloneExpr(&e) : &e;
    target->inputs = std::move(newInputs);
    return target;
}

Rule Get_Normalize_Flatten_Rule() {
    return Rule{&Match_Flatten, &Rewrite_Flatten, Stage_Normalize};
}

} // namespace BitFlow::Core::Rules::Normalize