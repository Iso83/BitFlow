#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
// a & ... & 0 → 0
static bool Match_AndZeroDominance(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::And)
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const && store->isFalse(in))
            return true;
    }

    return false;
}

// a | ... | 0 → remove 0
static bool Match_OrZeroIdentity(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Or)
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const && store->isFalse(in))
            return true;
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_AndZeroDominance(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    return ctx.replace(id, store->makeFalse(e.bitWidth).id);
}

static ExprId Rewrite_OrZeroIdentity(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    ExprInputs newInputs;
    newInputs.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (!(exprIn.op == OpType::Const && store->isFalse(in)))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return ctx.replace(id, store->makeFalse(e.bitWidth).id);

    if (newInputs.size() == 1)
        return ctx.replace(id, newInputs[0]);

    return ctx.replace(id, store->create(e.op, std::move(newInputs), e.bitWidth).id);
}

static ExprId Rewrite_XorZero(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    ExprInputs newInputs;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const && store->isFalse(in))
            continue;

        newInputs.push_back(in);
    }

    if (newInputs.empty())
        return ctx.replace(id, store->makeFalse(e.bitWidth).id);

    if (newInputs.size() == 1)
        return ctx.replace(id, newInputs[0]);

    return ctx.replace(id, store->create(e.op, std::move(newInputs), e.bitWidth).id);
}
#pragma endregion

Rule Get_AndZeroDominance_Rule() {
    return Rule{AndZeroDominance, &Match_AndZeroDominance, &Rewrite_AndZeroDominance, {Normalize::Flatten}};
}

Rule Get_OrZeroIdentity_Rule() {
    return Rule{OrZeroIdentity, &Match_OrZeroIdentity, &Rewrite_OrZeroIdentity, {Normalize::Flatten}};
}

Rule Get_XorZero_Rule() {
    return Rule{XorZero, &Match_Zero<OpType::Xor>, &Rewrite_XorZero, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
