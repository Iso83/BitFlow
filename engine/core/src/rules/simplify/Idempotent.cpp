#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Simplify {

using Expr = AST::Expr;
using OpType = AST::OpType;

#pragma region Match
static bool Match_Idempotent(const Expr& e) {
    if (e.op != AST::OpType::And && e.op != AST::OpType::Or)
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (size_t i = 0; i < e.inputs.size(); ++i) {
        for (size_t j = i + 1; j < e.inputs.size(); ++j) {
            if (e.inputs[i]->id == e.inputs[j]->id)
                return true;
        }
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static Expr* Rewrite_Idempotent(Expr& e) {
    std::vector<Expr*> unique;

    for (Expr* in : e.inputs) {
        bool exists = false;
        for (Expr* u : unique) {
            if (u->id == in->id) {
                exists = true;
                break;
            }
        }

        if (!exists)
            unique.push_back(in);
    }

    if (unique.size() == 1)
        return unique[0];

    e.inputs = std::move(unique);
    return &e;
}
#pragma endregion

Rule Get_Idempotent_Rule() {
    return Rule{RuleId::Simplify_Idempotent,
                &Match_Idempotent,
                &Rewrite_Idempotent,
                Stage_Simplify,
                {RuleId::Normalize_Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify