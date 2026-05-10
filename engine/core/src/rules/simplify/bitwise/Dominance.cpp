#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
// a & ... & 0 → 0
static bool Match_And_ZeroDominance(const ExprStore* store, ExprId id) {
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
static bool Match_And_OneIdentity(const ExprStore* store, ExprId id) {
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
static bool Match_Or_OneDominance(const ExprStore* store, ExprId id) {
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
static bool Match_Or_ZeroIdentity(const ExprStore* store, ExprId id) {
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
static ExprId Rewrite_And_ZeroDominance(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    return store->makeFalse(e.bitWidth).id;
}

static ExprId Rewrite_And_OneIdentity(ExprStore* store, ExprId id) {
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

static ExprId Rewrite_Or_OneDominance(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    return store->makeTrue(e.bitWidth).id;
}

static ExprId Rewrite_Or_ZeroIdentity(ExprStore* store, ExprId id) {
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

Rule Get_And_Zero_Dominance_Rule() {
    return Rule{And_Zero_Dominance, &Match_And_ZeroDominance, &Rewrite_And_ZeroDominance, {Normalize::Flatten}};
}

Rule Get_And_One_Identity_Rule() {
    return Rule{And_One_Identity, &Match_And_OneIdentity, &Rewrite_And_OneIdentity, {Normalize::Flatten}};
}

Rule Get_Or_One_Dominance_Rule() {
    return Rule{Or_One_Dominance, &Match_Or_OneDominance, &Rewrite_Or_OneDominance, {Normalize::Flatten}};
}

Rule Get_Or_Zero_Identity_Rule() {
    return Rule{Or_Zero_Identity, &Match_Or_ZeroIdentity, &Rewrite_Or_ZeroIdentity, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
