#include "expression/ExprUtils.h"
#include "rules/RuleStage.h"

#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
// a & ... & 0 → 0
static bool Match_And_ZeroDominance(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::And)
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (exprIn.op == OpType::Const && IsFalse(store, in))
            return true;
    }

    return false;
}

// a & ... & 1 → remove 1
static bool Match_And_OneIdentity(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::And)
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (exprIn.op == OpType::Const && IsTrue(store, in))
            return true;
    }

    return false;
}

// a | ... | 1 → 1
static bool Match_Or_OneDominance(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::Or)
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (exprIn.op == OpType::Const && IsTrue(store, in))
            return true;
    }

    return false;
}

// a | ... | 0 → remove 0
static bool Match_Or_ZeroIdentity(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::Or)
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (exprIn.op == OpType::Const && IsFalse(store, in))
            return true;
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_And_ZeroDominance(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    return store->makeFalse(e.bitWidth).id;
}

static ExprId Rewrite_And_OneIdentity(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    std::vector<ExprId> newInputs;
    newInputs.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (!(exprIn.op == OpType::Const && IsTrue(store, in)))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return store->makeTrue(e.bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(e.op, std::move(newInputs), e.bitWidth).id;
}

static ExprId Rewrite_Or_OneDominance(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    return store->makeTrue(e.bitWidth).id;
}

static ExprId Rewrite_Or_ZeroIdentity(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    std::vector<ExprId> newInputs;
    newInputs.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (!(exprIn.op == OpType::Const && IsFalse(store, in)))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return store->makeFalse(e.bitWidth).id;

    if (newInputs.size() == 1)
        return newInputs[0];

    return store->create(e.op, std::move(newInputs), e.bitWidth).id;
}
#pragma endregion

Rule Get_And_ZeroDominance_Rule() {
    return Rule{RuleId::Simplify_AndZeroDominance, &Match_And_ZeroDominance, &Rewrite_And_ZeroDominance, Stage_Simplify,
                {RuleId::Normalize_Flatten},       RuleFlags::None,          "Simplify_AndZeroDominance"};
}

Rule Get_And_OneIdentity_Rule() {
    return Rule{RuleId::Simplify_AndOneIdentity, &Match_And_OneIdentity, &Rewrite_And_OneIdentity, Stage_Simplify,
                {RuleId::Normalize_Flatten},     RuleFlags::None,        "Simplify_AndOneIdentity"};
}

Rule Get_Or_OneDominance_Rule() {
    return Rule{RuleId::Simplify_OrOneDominance, &Match_Or_OneDominance, &Rewrite_Or_OneDominance, Stage_Simplify,
                {RuleId::Normalize_Flatten},     RuleFlags::None,        "Simplify_OrOneDominance"};
}

Rule Get_Or_ZeroIdentity_Rule() {
    return Rule{RuleId::Simplify_OrZeroIdentity, &Match_Or_ZeroIdentity, &Rewrite_Or_ZeroIdentity, Stage_Simplify,
                {RuleId::Normalize_Flatten},     RuleFlags::None,        "Simplify_OrZeroIdentity"};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
