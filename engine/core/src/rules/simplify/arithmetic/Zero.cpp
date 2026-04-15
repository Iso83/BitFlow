#include "expression/ExprClone.h"
#include "rules/RuleCommon.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using Expr = AST::Expr;

static Expr* Rewrite_Add_Zero(Expr& e) {
    std::vector<Expr*> newInputs;

    for (Expr* in : e.inputs) {
        if (!(in->isConst() && in->constValue == 0))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return Expression::ConstPool::Get(0);

    if (newInputs.size() == 1)
        return newInputs[0];

    Expr* target = Expression::CloneExpr(&e);

    target->inputs = std::move(newInputs);
    return target;
}

static Expr* Rewrite_Mul_Zero(Expr&) {
    return Expression::ConstPool::Get(0);
}

static bool Match_Sub_Zero(const Expr& e) {
    if (e.op != AST::OpType::Sub)
        return false;

    if (e.inputs.size() != 2)
        return false;

    const Expr* rhs = e.inputs[1];
    return rhs->isConst() && rhs->constValue == 0;
}

static Expr* Rewrite_Sub_Zero(Expr& e) {
    return e.inputs[0];
}

Rule Get_Add_Zero_Rule() {
    return Rule{RuleId::Simplify_AddZero,
                &Match_Zero<AST::OpType::Add>,
                &Rewrite_Add_Zero,
                Stage_Simplify,
                {RuleId::Normalize_Flatten}};
}

Rule Get_Mul_Zero_Rule() {
    return Rule{RuleId::Simplify_MulZero,
                &Match_Zero<AST::OpType::Mul>,
                &Rewrite_Mul_Zero,
                Stage_Simplify,
                {RuleId::Normalize_Flatten}};
}

Rule Get_Sub_Zero_Rule() {
    return Rule{RuleId::Simplify_SubZero,
                &Match_Sub_Zero,
                &Rewrite_Sub_Zero,
                Stage_Simplify,
                {RuleId::Normalize_Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
