#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
// a & ... & 1 → remove 1
static bool Match_AndOneIdentity(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::And)
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const && store->isTrue(in))
            return true;
    }

    return false;
}

// a | ... | 1 → 1
static bool Match_OrOneDominance(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Or)
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const && store->isTrue(in))
            return true;
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_AndOneIdentity(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    ExprInputs newInputs;
    newInputs.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (!(exprIn.op == OpType::Const && store->isTrue(in)))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return ctx.replace(id, store->makeTrue(e.bitWidth).id);

    if (newInputs.size() == 1)
        return ctx.replace(id, newInputs[0]);

    return ctx.replace(id, store->create(e.op, std::move(newInputs), e.bitWidth).id);
}

static ExprId Rewrite_OrOneDominance(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    return ctx.replace(id, store->makeTrue(e.bitWidth).id);
}
#pragma endregion

Rule Get_AndOneIdentity_Rule() {
    return Rule{AndOneIdentity, &Match_AndOneIdentity, &Rewrite_AndOneIdentity, {Normalize::Flatten}};
}

Rule Get_OrOneDominance_Rule() {
    return Rule{OrOneDominance, &Match_OrOneDominance, &Rewrite_OrOneDominance, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
