#include "rules/RuleStage.h"

#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using Expr = Expression::Expr;
using OpType = Expression::OpType;

static bool Match_Mul_One(const Expr& e) {
    if (e.op != OpType::Mul)
        return false;

    for (const Expr* in : e.inputs) {
        if (in->op == OpType::Const && in->constValue == 1)
            return true;
    }

    return false;
}

static Expr* Rewrite_Mul_One(Expr& e) {
    std::vector<Expr*> newInputs;
    newInputs.reserve(e.inputs.size());

    for (Expr* in : e.inputs) {
        if (!(in->op == OpType::Const && in->constValue == 1))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return Expression::ConstPool::Get(1);

    if (newInputs.size() == 1)
        return newInputs[0];

    Expr* target = Expression::CloneExpr(&e);
    target->inputs = std::move(newInputs);
    return target;
}

static bool Match_Div_One(const Expr& e) {
    if (e.op != OpType::Div)
        return false;

    if (e.inputs.size() != 2)
        return false;

    const Expr* rhs = e.inputs[1];
    return rhs->op == OpType::Const && rhs->constValue == 1;
}

static Expr* Rewrite_Div_One(Expr& e) {
    return e.inputs[0];
}

Rule Get_Mul_One_Rule() {
    return Rule{RuleId::Simplify_MulOne,     &Match_Mul_One,        &Rewrite_Mul_One, Stage_Simplify,
                {RuleId::Normalize_Flatten}, RuleFlags::Arithmetic, "Simplify_MulOne"};
}

Rule Get_Div_One_Rule() {
    return Rule{RuleId::Simplify_DivOne,     &Match_Div_One,        &Rewrite_Div_One, Stage_Simplify,
                {RuleId::Normalize_Flatten}, RuleFlags::Arithmetic, "Simplify_DivOne"};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
