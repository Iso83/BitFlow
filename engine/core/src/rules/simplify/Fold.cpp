#include "expression/ExprClone.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify {

using Expr = AST::Expr;

#pragma region Match
static bool Match_Add_Fold(const Expr& e) {
    if (e.op != AST::OpType::Add)
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
static Expr* Rewrite_Add_Fold(Expr& e) {
    uint32_t acc = 0;
    std::vector<Expr*> nonConst;

    for (Expr* in : e.inputs) {
        if (in->isConst())
            acc += in->constValue;
        else
            nonConst.push_back(in);
    }

    if (acc != 0) {
        Expr* c = Expression::ConstPool::Get(acc);
        nonConst.push_back(c);
    }

    if (nonConst.empty())
        return Expression::ConstPool::Get(0);

    if (nonConst.size() == 1)
        return nonConst[0];

    Expr* target = Expression::CloneExpr(&e);
    target->inputs = std::move(nonConst);
    return target;
}

#pragma endregion

Rule Get_Add_Fold_Rule() {
    return Rule{
        RuleId::Simplify_AddFold, &Match_Add_Fold, &Rewrite_Add_Fold, Stage_Simplify, {RuleId::Normalize_Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify
