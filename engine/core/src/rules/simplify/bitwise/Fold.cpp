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

static bool Match_Xor_Fold(const Expr& e) {
    if (e.op != AST::OpType::Xor)
        return false;

    if (e.inputs.size() < 2)
        return false;

    int constCount = 0;

    for (const Expr* in : e.inputs) {
        if (in->isConst())
            constCount++;
    }

    return constCount >= 2;
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

static Expr* Rewrite_Xor_Fold(Expr& e) {
    uint32_t acc = 0;
    std::vector<Expr*> nonConst;
    nonConst.reserve(e.inputs.size());

    for (Expr* in : e.inputs) {
        if (in->isConst())
            acc ^= in->constValue;
        else
            nonConst.push_back(in);
    }

    if (acc != 0)
        nonConst.push_back(Expression::ConstPool::Get(acc));

    if (nonConst.empty())
        return Expression::ConstPool::Get(0);

    if (nonConst.size() == 1)
        return nonConst[0];

    Expr* target = Expression::CloneExpr(&e);
    target->inputs = std::move(nonConst);
    return target;
}
#pragma endregion

Rule Get_And_Fold_Rule() {
    return Rule{RuleId::Simplify_AndFold,    &Match_And_Fold, &Rewrite_And_Fold, Stage_Simplify,
                {RuleId::Normalize_Flatten}, RuleFlags::None, "Simplify_AndFold"};
}

Rule Get_Or_Fold_Rule() {
    return Rule{RuleId::Simplify_OrFold,     &Match_Or_Fold,  &Rewrite_Or_Fold, Stage_Simplify,
                {RuleId::Normalize_Flatten}, RuleFlags::None, "Simplify_OrFold"};
}

Rule Get_Xor_Fold_Rule() {
    return Rule{RuleId::Simplify_XorFold,    &Match_Xor_Fold, &Rewrite_Xor_Fold, Stage_Simplify,
                {RuleId::Normalize_Flatten}, RuleFlags::None, "Simplify_XorFold"};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
