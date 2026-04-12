#include "expression/ExprClone.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/Rule.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace BitFlow::Core::Rules::Simplify {

using Expr = AST::Expr;
using OpType = AST::OpType;

#pragma region Match
static bool Match_Idempotent(const Expr& e) {
    if (e.op != OpType::And && e.op != OpType::Or)
        return false;

    if (e.inputs.size() < 2)
        return false;

    std::unordered_map<uint32_t, int> counts;
    counts.reserve(e.inputs.size());

    for (const Expr* in : e.inputs) {
        const uint32_t key = in->id.value();
        counts[key]++;

        if (counts[key] >= 2)
            return true;
    }

    return false;
}

static bool Match_And_Idempotent(const Expr& e) {
    if (e.op != AST::OpType::And || e.inputs.size() < 2)
        return false;

    std::unordered_set<const Expr*> seen;

    for (auto* in : e.inputs) {
        if (!seen.insert(in).second)
            return true;
    }
    return false;
}
#pragma endregion

#pragma region Rewrite
static Expr* Rewrite_Idempotent(Expr& e) {
    std::vector<Expr*> unique;
    unique.reserve(e.inputs.size());

    std::unordered_map<uint32_t, bool> seen;
    seen.reserve(e.inputs.size());

    for (Expr* in : e.inputs) {
        const uint32_t key = in->id.value();

        if (seen[key])
            continue;

        seen[key] = true;
        unique.push_back(in);
    }

    if (unique.size() == 1)
        return unique[0];

    Expr* target = e.frozen ? Expression::CloneExpr(&e) : &e;
    target->inputs = std::move(unique);
    return target;
}

static Expr* Rewrite_And_Idempotent(Expr& e) {
    std::unordered_set<const Expr*> seen;
    std::vector<Expr*> unique;

    unique.reserve(e.inputs.size());

    for (auto* in : e.inputs) {
        if (seen.insert(in).second)
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

Rule Get_And_Idempotent_Rule() {
    return Rule{RuleId::Simplify_And_Idempotent,
                &Match_And_Idempotent,
                &Rewrite_And_Idempotent,
                Stage_Simplify,
                {RuleId::Normalize_Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify