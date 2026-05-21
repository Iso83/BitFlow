#pragma once

#include <BitFlow/core/expression/ExprStore.h>

namespace BitFlow::Core::Expression {

inline bool IsConstFalse(const ExprStore* store, Ids::ExprId id) {
    const Expr& e = (*store)[id];
    return e.op == OpType::Const && store->isFalse(id);
}

#pragma region Matching
inline bool ContainsExpr(const ExprStore* store, Ids::ExprId id, Ids::ExprId target) {
    if (store->structuralEquivalent(id, target))
        return true;

    const Expr& e = (*store)[id];

    for (auto in : e.inputs) {
        if (store->structuralEquivalent(in, target))
            return true;
    }

    return false;
}

template <OpType Op> inline bool Match_Zero(const ExprStore* store, Ids::ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != Op)
        return false;

    if (e.inputs.empty())
        return false;

    for (auto in : e.inputs) {
        if (IsConstFalse(store, in))
            return true;
    }

    return false;
}
#pragma endregion

inline int CompareExprCanonical(const ExprStore* store, Ids::ExprId a, Ids::ExprId b) {
    if (a == b)
        return 0;

    const Expr& exprA = (*store)[a];
    const Expr& exprB = (*store)[b];

    // --- constants first ---
    if (exprA.op == OpType::Const && exprB.op != OpType::Const)
        return -1;

    if (exprA.op != OpType::Const && exprB.op == OpType::Const)
        return 1;

    // --- variables second ---
    if (exprA.op == OpType::Var && exprB.op != OpType::Var)
        return -1;

    if (exprA.op != OpType::Var && exprB.op == OpType::Var)
        return 1;

    // --- op type ---
    if (exprA.op != exprB.op)
        return static_cast<int>((OpType)exprA.op) < static_cast<int>((OpType)exprB.op) ? -1 : 1;

    // --- constants ---
    if (exprA.op == OpType::Const) {
        if (exprA.knownValue != exprB.knownValue)
            return exprA.knownValue < exprB.knownValue ? -1 : 1;

        return 0;
    }

    // --- variables ---
    if (exprA.op == OpType::Var)
        return a.value() < b.value() ? -1 : 1;

    // --- arity ---
    if (exprA.inputs.size() != exprB.inputs.size())
        return exprA.inputs.size() < exprB.inputs.size() ? -1 : 1;

    // --- recursive compare ---
    for (size_t i = 0; i < exprA.inputs.size(); ++i) {
        const int cmp = CompareExprCanonical(store, exprA.inputs[i], exprB.inputs[i]);

        if (cmp != 0)
            return cmp;
    }

    return 0;
}

inline bool CanonicalExprLess(const ExprStore* store, Ids::ExprId a, Ids::ExprId b) {
    return CompareExprCanonical(store, a, b) < 0;
}

} // namespace BitFlow::Core::Expression