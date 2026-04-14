#include "ast/ExprIntern.h"
#include "expression/ExprFactory.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using Expr = AST::Expr;

#pragma region Match
static bool Match_Not(const Expr& e) {
    if (e.op != AST::OpType::Not)
        return false;

    if (e.inputs.size() != 1)
        return false;

    const Expr* in = e.inputs[0];

    if (in->op == AST::OpType::Not && in->inputs.size() == 1)
        return true;

    if (in->isConst())
        return true;

    return false;
}

static bool Match_NotPushdown(const Expr& e) {
    if (e.op != AST::OpType::Not)
        return false;

    if (e.inputs.size() != 1)
        return false;

    const Expr* in = e.inputs[0];

    if (!(in->op == AST::OpType::And || in->op == AST::OpType::Or))
        return false;

    bool allNot = true;
    for (const Expr* child : in->inputs) {
        if (child->op != AST::OpType::Not) {
            allNot = false;
            break;
        }
    }

    return !allNot;
}
#pragma endregion

#pragma region Rewrite
static Expr* Rewrite_Not(Expr& e) {
    Expr* in = e.inputs[0];

    if (in->op == AST::OpType::Not && in->inputs.size() == 1)
        return in->inputs[0];

    if (in->isConst())
        return Expression::ConstPool::Get(~in->constValue);

    return nullptr;
}

static Expr* Rewrite_NotPushdown(Expr& e) {
    Expr* in = e.inputs[0];

    AST::OpType newOp = (in->op == AST::OpType::And) ? AST::OpType::Or : AST::OpType::And;

    std::vector<Expr*> newInputs;
    newInputs.reserve(in->inputs.size());

    for (Expr* child : in->inputs) {
        auto* n = Expression::MakeOpInterned(AST::OpType::Not, {child});
        newInputs.push_back(n);
    }

    auto* target = Expression::MakeOpInterned(newOp, std::move(newInputs));
    return target;
}
#pragma endregion

Rule Get_Not_Rule() {
    return Rule{RuleId::Simplify_Not, &Match_Not, &Rewrite_Not, Stage_Simplify};
}

Rule Get_NotPushdown_Rule() {
    return Rule{RuleId::Simplify_NotPushdown, &Match_NotPushdown, &Rewrite_NotPushdown, Stage_Simplify_Pushdown};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
