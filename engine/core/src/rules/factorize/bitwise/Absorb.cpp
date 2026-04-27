#include "rules/RuleStage.h"

#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Factorize::Bitwise {

using Expr = Expression::Expr;
using OpType = Expression::OpType;

static bool ContainsExpr(const Expr* e, const Expr* target) {
    if (e == target)
        return true;

    for (const Expr* in : e->inputs) {
        if (in == target)
            return true;
    }

    return false;
}

#pragma region Match
static bool Match_And_Absorb(const Expr& e) {
    if (e.op != OpType::And)
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (Expr* a : e.inputs) {
        for (Expr* b : e.inputs) {
            if (b->op != OpType::Or)
                continue;

            for (Expr* inner : b->inputs) {
                if (inner->id == a->id)
                    return true;
            }
        }
    }

    return false;
}

static bool Match_Or_Absorb(const Expr& e) {
    if (e.op != OpType::Or)
        return false;

    if (e.inputs.size() < 2)
        return false;

    for (Expr* a : e.inputs) {
        for (Expr* b : e.inputs) {
            if (b->op != OpType::And)
                continue;

            for (Expr* inner : b->inputs) {
                if (inner->id == a->id)
                    return true;
            }
        }
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static Expr* Rewrite_And_Absorb(Expr& e) {
    if (e.op != OpType::And)
        return nullptr;

    for (auto* a : e.inputs) {
        for (auto* b : e.inputs) {
            if (a == b)
                continue;

            if (b->op == OpType::Or && ContainsExpr(b, a))
                return a;
        }
    }

    return nullptr;
}

static Expr* Rewrite_Or_Absorb(Expr& e) {
    if (e.op != OpType::Or)
        return nullptr;

    for (auto* a : e.inputs) {
        for (auto* b : e.inputs) {
            if (a == b)
                continue;

            if (b->op == OpType::And && ContainsExpr(b, a))
                return a;
        }
    }

    return nullptr;
}
#pragma endregion

Rule Get_And_Absorb_Rule() {
    return Rule{RuleId::Factorize_AndAbsorb, &Match_And_Absorb,      &Rewrite_And_Absorb,  Stage_Factorize,
                {RuleId::Normalize_Flatten}, RuleFlags::Factorizing, "Factorize_AndAbsorb"};
}

Rule Get_Or_Absorb_Rule() {
    return Rule{RuleId::Factorize_OrAbsorb,  &Match_Or_Absorb,       &Rewrite_Or_Absorb,  Stage_Factorize,
                {RuleId::Normalize_Flatten}, RuleFlags::Factorizing, "Factorize_OrAbsorb"};
}

} // namespace BitFlow::Core::Rules::Factorize::Bitwise