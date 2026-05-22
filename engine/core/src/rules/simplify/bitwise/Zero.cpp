#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
// a & ... & 0 → 0
static bool Match_AndZero(const ExprStore* store, ExprId id) {
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
static bool Match_OrZero(const ExprStore* store, ExprId id) {
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
static ExprId Rewrite_AndZero(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    return store->makeFalse(e.bitWidth).id;
}

static ExprId Rewrite_OrZero(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    std::vector<ExprId> newInputs;
    newInputs.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (!(exprIn.op == OpType::Const && store->isFalse(in)))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return store->makeFalse(e.bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(e.op, std::move(newInputs), e.bitWidth).id;
}

static ExprId Rewrite_XorZero(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    std::vector<ExprId> newInputs;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const && store->isFalse(in))
            continue;

        newInputs.push_back(in);
    }

    if (newInputs.empty())
        return store->makeFalse(e.bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(e.op, std::move(newInputs), e.bitWidth).id;
}
#pragma endregion

Rule Get_AndZero_Rule() {
    return Rule{AndZero, &Match_AndZero, &Rewrite_AndZero, {Normalize::Flatten}};
}

Rule Get_OrZero_Rule() {
    return Rule{OrZero, &Match_OrZero, &Rewrite_OrZero, {Normalize::Flatten}};
}

Rule Get_XorZero_Rule() {
    return Rule{XorZero, &Match_Zero<OpType::Xor>, &Rewrite_XorZero, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
