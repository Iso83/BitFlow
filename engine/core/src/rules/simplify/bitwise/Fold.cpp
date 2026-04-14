#include "expression/ExprClone.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using Expr = AST::Expr;

#pragma region Match
static bool Match_And_Fold(const Expr& e) {
    if (e.op != AST::OpType::And)
        return false;

    if (e.inputs.size() < 2)
        return false;

    return true;
}

static bool Match_Or_Fold(const Expr& e) {
    if (e.op != AST::OpType::Or)
        return false;

    if (e.inputs.size() < 2)
        return false;

    return true;
}
#pragma endregion

#pragma region Rewrite
static Expr* Rewrite_And_Fold(Expr& e) {
    std::vector<Expr*> newInputs;

    for (Expr* in : e.inputs) {
        if (in->isConst() && in->constValue == 0)
            return Expression::ConstPool::Get(0);

        if (in->isConst() && in->constValue == 1)
            continue;

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

static Expr* Rewrite_Or_Fold(Expr& e) {
    std::vector<Expr*> newInputs;

    for (Expr* in : e.inputs) {
        if (in->isConst() && in->constValue == 1)
            return Expression::ConstPool::Get(1);

        if (in->isConst() && in->constValue == 0)
            continue;

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
#pragma endregion

Rule Get_And_Fold_Rule() {
    return Rule{
        RuleId::Simplify_AndFold, &Match_And_Fold, &Rewrite_And_Fold, Stage_Simplify, {RuleId::Normalize_Flatten}};
}

Rule Get_Or_Fold_Rule() {
    return Rule{RuleId::Simplify_OrFold, &Match_Or_Fold, &Rewrite_Or_Fold, Stage_Simplify, {RuleId::Normalize_Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
