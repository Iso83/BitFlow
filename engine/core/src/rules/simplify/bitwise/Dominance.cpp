#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
// a & ... & 0 → 0
static bool Match_AndZeroDominance(const ExprStore* store, ExprId id) {
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

// a | ... | 0 → remove 0
static bool Match_OrZeroIdentity(const ExprStore* store, ExprId id) {
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
static ExprId Rewrite_AndZeroDominance(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    return store->makeFalse(e.bitWidth).id;
}

static ExprId Rewrite_AndOneIdentity(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    std::vector<ExprId> newInputs;
    newInputs.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (!(exprIn.op == OpType::Const && store->isTrue(in)))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return store->makeTrue(e.bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(e.op, std::move(newInputs), e.bitWidth).id;
}

static ExprId Rewrite_OrOneDominance(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    return store->makeTrue(e.bitWidth).id;
}

static ExprId Rewrite_OrZeroIdentity(ExprStore* store, ExprId id) {
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
#pragma endregion

Rule Get_AndZeroDominance_Rule() {
    return Rule{AndZeroDominance, &Match_AndZeroDominance, &Rewrite_AndZeroDominance, {Normalize::Flatten}};
}

Rule Get_AndOneIdentity_Rule() {
    return Rule{AndOneIdentity, &Match_AndOneIdentity, &Rewrite_AndOneIdentity, {Normalize::Flatten}};
}

Rule Get_OrOneDominance_Rule() {
    return Rule{OrOneDominance, &Match_OrOneDominance, &Rewrite_OrOneDominance, {Normalize::Flatten}};
}

Rule Get_OrZeroIdentity_Rule() {
    return Rule{OrZeroIdentity, &Match_OrZeroIdentity, &Rewrite_OrZeroIdentity, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
