#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Factorize::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Match
static bool Match_And_Absorb(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::And || e.inputs.size() < 2)
        return false;

    for (auto a : e.inputs) {
        for (auto b : e.inputs) {
            if (a == b)
                continue;

            const Expr& exprB = store->get(b);

            if (exprB.op == OpType::Or && ContainsExpr(store, b, a))
                return true;
        }
    }

    return false;
}

static bool Match_Or_Absorb(const ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    if (e.op != OpType::Or || e.inputs.size() < 2)
        return false;

    for (auto a : e.inputs) {
        for (auto b : e.inputs) {
            if (a == b)
                continue;

            const Expr& exprB = store->get(b);

            if (exprB.op == OpType::And && ContainsExpr(store, b, a))
                return true;
        }
    }

    return false;
}
#pragma endregion

#pragma region Rewrite
static ExprId Rewrite_And_Absorb(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    _ASSERT(e.op == OpType::And);

    for (auto a : e.inputs) {
        for (auto b : e.inputs) {
            if (a == b)
                continue;

            const Expr& exprB = store->get(b);

            if (exprB.op == OpType::Or && ContainsExpr(store, b, a))
                return a;
        }
    }

    _ASSERT(false);
    return id;
}

static ExprId Rewrite_Or_Absorb(ExprStore* store, ExprId id) {
    const Expr& e = store->get(id);

    _ASSERT(e.op == OpType::Or);

    for (auto a : e.inputs) {
        for (auto b : e.inputs) {
            if (a == b)
                continue;

            const Expr& exprB = store->get(b);

            if (exprB.op == OpType::And && ContainsExpr(store, b, a))
                return a;
        }
    }

    _ASSERT(false);
    return id;
}
#pragma endregion

Rule Get_And_Absorb_Rule() {
    return Rule{And_Absorb, &Match_And_Absorb, &Rewrite_And_Absorb, {Normalize::Flatten}};
}

Rule Get_Or_Absorb_Rule() {
    return Rule{Or_Absorb, &Match_Or_Absorb, &Rewrite_Or_Absorb, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Factorize::Bitwise