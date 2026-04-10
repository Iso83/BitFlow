#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Factorize {

using Expr = AST::Expr;
using OpType = AST::OpType;

#pragma region Match
static bool Match_And_Distribute(const Expr& e) {
    if (e.op != OpType::And)
        return false;

    if (e.inputs.size() != 2)
        return false;

    Expr* a = e.inputs[0];
    Expr* b = e.inputs[1];

    return (a->op == OpType::Or && a->inputs.size() == 2) || (b->op == OpType::Or && b->inputs.size() == 2);
}
#pragma endregion

#pragma region Rewrite
static Expr* Rewrite_And_Distribute(Expr& e) {
    Expr* a = e.inputs[0];
    Expr* b = e.inputs[1];

    Expr* orNode = (a->op == OpType::Or) ? a : b;
    Expr* other = (a->op == OpType::Or) ? b : a;

    Expr* result = new Expr{};
    result->op = OpType::Or;

    for (Expr* term : orNode->inputs) {
        Expr* andNode = new Expr{};
        andNode->op = OpType::And;
        andNode->inputs = {other, term};

        result->inputs.push_back(andNode);
    }

    return result;
}
#pragma endregion

Rule Get_And_Distribute_Rule() {
    return Rule{RuleId::Factorize_AndDistribute,
                &Match_And_Distribute,
                &Rewrite_And_Distribute,
                Stage_Factorize,
                {RuleId::Normalize_Flatten}};
}

} // namespace BitFlow::Core::Rules::Factorize