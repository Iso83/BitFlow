#include "rules/RuleStage.h"

#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using Expr = Expression::ExprOld;
using OpType = Expression::OpType;

#pragma region Match
// a & ... & 0 → 0
static bool Match_And_ZeroDominance(const Expr& e) {
    if (e.op != OpType::And)
        return false;

    for (const Expr* in : e.inputs) {
        if (in->op == OpType::Const && in->constValue == 0)
            return true;
    }

    return false;
}

// a & ... & 1 → remove 1
static bool Match_And_OneIdentity(const Expr& e) {
    if (e.op != OpType::And)
        return false;

    for (const Expr* in : e.inputs) {
        if (in->op == OpType::Const && in->constValue == 1)
            return true;
    }

    return false;
}

// a | ... | 1 → 1
static bool Match_Or_OneDominance(const Expr& e) {
    if (e.op != OpType::Or)
        return false;

    for (const Expr* in : e.inputs) {
        if (in->op == OpType::Const && in->constValue == 1)
            return true;
    }

    return false;
}

// a | ... | 0 → remove 0
static bool Match_Or_ZeroIdentity(const Expr& e) {
    if (e.op != OpType::Or)
        return false;

    for (const Expr* in : e.inputs) {
        if (in->op == OpType::Const && in->constValue == 0)
            return true;
    }

    return false;
}
#pragma endregion

#pragma region Rewrite

static Expr* Rewrite_And_ZeroDominance(Expr&) {
    return Expression::ConstPool::Get(0);
}

static Expr* Rewrite_And_OneIdentity(Expr& e) {
    std::vector<Expr*> newInputs;
    newInputs.reserve(e.inputs.size());

    for (Expr* in : e.inputs) {
        if (!(in->op == OpType::Const && in->constValue == 1))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return Expression::ConstPool::Get(1);

    if (newInputs.size() == 1)
        return newInputs[0];

    Expr* target = Expression::CloneExpr(&e);
    target->inputs = std::move(newInputs);
    return target;
}

static Expr* Rewrite_Or_OneDominance(Expr&) {
    return Expression::ConstPool::Get(1);
}

static Expr* Rewrite_Or_ZeroIdentity(Expr& e) {
    std::vector<Expr*> newInputs;
    newInputs.reserve(e.inputs.size());

    for (Expr* in : e.inputs) {
        if (!(in->op == OpType::Const && in->constValue == 0))
            newInputs.push_back(in);
    }

    if (newInputs.empty())
        return Expression::ConstPool::Get(0);

    if (newInputs.size() == 1)
        return newInputs[0];

    Expr* target = Expression::CloneExpr(&e);
    target->inputs = std::move(newInputs);
    return target;
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
