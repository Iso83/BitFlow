#pragma once

#include <BitFlow/engine/core/expression/ExprStore.h>

namespace BitFlow::Engine::Core::Expression {

inline bool IsConstFalse(const ExprStore* store, Ids::ExprId id) {
    const Expr& e = (*store)[id];
    return e.op == OpType::Const && store->isFalse(id);
}

#pragma region Match
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

template <OpType Op> inline bool Match_Zero(const ExprStore* store, const ExprNameMap* names, Ids::ExprId id) {
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

template <OpType Op, bool MatchTrue, bool MatchFalse>
inline bool HasBooleanConstantInput(const ExprStore* store, const ExprNameMap* names, Ids::ExprId id) {
    static_assert(MatchFalse || MatchTrue);

    const Expr& e = (*store)[id];

    if (e.op != Op || e.inputs.size() < 2)
        return false;

    for (auto in : e.inputs) {

        const Expr& exprIn = (*store)[in];

        if (exprIn.op != OpType::Const)
            continue;

        if constexpr (MatchTrue)
            if (store->isTrue(in))
                return true;

        if constexpr (MatchFalse)
            if (store->isFalse(in))
                return true;
    }

    return false;
}
#pragma endregion

inline int CompareExprCanonical(const ExprStore* store, const ExprNameMap* names, Ids::ExprId a, Ids::ExprId b) {
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
    if (exprA.op == OpType::Var) {

        std::string_view nameA;
        std::string_view nameB;

        if (names != nullptr) {

            if (auto it = names->find(a); it != names->end())
                nameA = it->second;

            if (auto it = names->find(b); it != names->end())
                nameB = it->second;
        }

        if (nameA != nameB)
            return nameA < nameB ? -1 : 1;

        return a.value() < b.value() ? -1 : 1;
    }

    // --- arity ---
    if (exprA.inputs.size() != exprB.inputs.size())
        return exprA.inputs.size() < exprB.inputs.size() ? -1 : 1;

    // --- recursive compare ---
    for (size_t i = 0; i < exprA.inputs.size(); ++i) {
        const int cmp = CompareExprCanonical(store, names, exprA.inputs[i], exprB.inputs[i]);

        if (cmp != 0)
            return cmp;
    }

    return 0;
}

inline bool CanonicalExprLess(const ExprStore* store, const ExprNameMap* names, Ids::ExprId a, Ids::ExprId b) {
    return CompareExprCanonical(store, names, a, b) < 0;
}

} // namespace BitFlow::Engine::Core::Expression
