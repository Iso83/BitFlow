#include "rules/RuleStage.h"

#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using Expr = Expression::Expr;
using OpType = Expression::OpType;

static bool Match_Neg_Neg(const Expr& e) {
    if (e.op != OpType::Neg || e.inputs.size() != 1)
        return false;

    const Expr* in = e.inputs[0];
    return in && in->op == OpType::Neg && in->inputs.size() == 1;
}

static Expr* Rewrite_Neg_Neg(Expr& e) {
    return e.inputs[0]->inputs[0];
}

Rule Get_Neg_Neg_Rule() {
    return Rule{RuleId::Simplify_NegNeg,     &Match_Neg_Neg,        &Rewrite_Neg_Neg, Stage_Simplify,
                {RuleId::Normalize_Flatten}, RuleFlags::Arithmetic, "Simplify_NegNeg"};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
