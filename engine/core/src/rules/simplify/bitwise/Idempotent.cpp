#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
static bool Match_Idempotent(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::And && e.op != OpType::Or)
        return false;

    if (e.inputs.size() < 2)
        return false;

    std::unordered_map<ExprId, int> counts;
    counts.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        counts[in]++;

        if (counts[in] >= 2)
            return true;
    }

    return false;
}

static bool Match_AndIdempotent(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::And || e.inputs.size() < 2)
        return false;

    std::unordered_set<ExprId> seen;

    for (auto in : e.inputs) {
        if (!seen.insert(in).second)
            return true;
    }
    return false;
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_Idempotent(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    ExprInputs unique;
    unique.reserve(e.inputs.size());

    std::unordered_map<ExprId, bool> seen;
    seen.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        if (seen[in])
            continue;

        seen[in] = true;
        unique.push_back(in);
    }

    if (unique.size() == 1)
        return ctx.replace(id, unique[0]);

    return ctx.replace(id, store->create(e.op, std::move(unique), e.bitWidth).id);
}

static ExprId Rewrite_AndIdempotent(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    std::unordered_set<ExprId> seen;
    ExprInputs unique;

    unique.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        if (seen.insert(in).second)
            unique.push_back(in);
    }

    if (unique.size() == 1)
        return ctx.replace(id, unique[0]);

    return ctx.replace(id, store->create(e.op, std::move(unique), e.bitWidth).id);
}
#pragma endregion

Rule Get_Idempotent_Rule() {
    return Rule{Idempotent, &Match_Idempotent, &Rewrite_Idempotent, {Normalize::Flatten}};
}

Rule Get_AndIdempotent_Rule() {
    return Rule{AndIdempotent, &Match_AndIdempotent, &Rewrite_AndIdempotent, {Normalize::Flatten}};
}
} // namespace BitFlow::Core::Rules::Simplify::Bitwise
