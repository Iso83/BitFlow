#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/expression/ConstPool.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Simplify {

using Expr = AST::Expr;
using OpType = AST::OpType;

#pragma region Match
static bool Match_Complement(const Expr& e) {
    if (e.op != AST::OpType::And && e.op != AST::OpType::Or)
        return false;

    if (e.inputs.size() != 2)
        return false;

    Expr* a = e.inputs[0];
    Expr* b = e.inputs[1];

    // a & ~a  /  a | ~a
    if (b->op == AST::OpType::Not && b->inputs.size() == 1)
        return (b->inputs[0]->id == a->id);

    if (a->op == AST::OpType::Not && a->inputs.size() == 1)
        return (a->inputs[0]->id == b->id);

    return false;
}
#pragma endregion

#pragma region Rewrite
static Expr* Rewrite_Complement(Expr& e) {
    if (e.op == AST::OpType::And)
        return Expression::ConstPool::Get(0);

    // OR
    return Expression::ConstPool::Get(~0u);
}
#pragma endregion

Rule Get_Complement_Rule() {
    return Rule{RuleId::Simplify_Complement,
                &Match_Complement,
                &Rewrite_Complement,
                Stage_Simplify,
                {RuleId::Normalize_Flatten, RuleId::Simplify_Idempotent}};
}

} // namespace BitFlow::Core::Rules::Simplify