#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Factorize::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
static bool Match_AndAbsorb(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::And || e.inputs.size() < 2)
        return false;

    for (auto a : e.inputs) {
        for (auto b : e.inputs) {
            if (a == b)
                continue;

            const Expr& exprB = (*store)[b];

            if (exprB.op == OpType::Or && ContainsExpr(store, b, a))
                return true;
        }
    }

    return false;
}

static bool Match_OrAbsorb(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::Or || e.inputs.size() < 2)
        return false;

    for (auto a : e.inputs) {
        for (auto b : e.inputs) {
            if (a == b)
                continue;

            const Expr& exprB = (*store)[b];

            if (exprB.op == OpType::And && ContainsExpr(store, b, a))
                return true;
        }
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_AndAbsorb(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    _ASSERT(e.op == OpType::And);

    for (auto a : e.inputs) {
        for (auto b : e.inputs) {
            if (a == b)
                continue;

            const Expr& exprB = (*store)[b];

            if (exprB.op == OpType::Or && ContainsExpr(store, b, a))
                return a;
        }
    }

    _ASSERT(false);
    return id;
}

static ExprId Rewrite_OrAbsorb(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    _ASSERT(e.op == OpType::Or);

    for (auto a : e.inputs) {
        for (auto b : e.inputs) {
            if (a == b)
                continue;

            const Expr& exprB = (*store)[b];

            if (exprB.op == OpType::And && ContainsExpr(store, b, a))
                return a;
        }
    }

    _ASSERT(false);
    return id;
}
#pragma endregion

Rule Get_AndAbsorb_Rule() {
    return Rule{AndAbsorb, &Match_AndAbsorb, &Rewrite_AndAbsorb, {Normalize::Flatten}};
}

Rule Get_OrAbsorb_Rule() {
    return Rule{OrAbsorb, &Match_OrAbsorb, &Rewrite_OrAbsorb, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Factorize::Bitwise