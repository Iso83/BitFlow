#pragma once

#include <BitFlow/core/expression/ExprStore.h>

namespace BitFlow::Core::Expression {

#pragma region Matching
inline bool ContainsExpr(const ExprStore* store, Ids::ExprId id, Ids::ExprId target) {
    if (id == target)
        return true;

    const Expr& e = store->get(id);

    for (auto in : e.inputs) {
        if (in == target)
            return true;
    }

    return false;
}

template <OpType Op> inline bool Match_Zero(const ExprStore* store, Ids::ExprId id) {
    const Expr& e = store->get(id);
    if (e.op != Op)
        return false;

    if (e.inputs.empty())
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = store->get(in);
        if (exprIn.op == OpType::Const && exprIn.inputs.empty() && exprIn.knownValue == 0)
            return true;
    }

    return false;
}
#pragma endregion

inline Ids::ExprId MakeXor(Expression::ExprStore* store, std::vector<Ids::ExprId>& terms,
                           Types::BitWidth bitWidth = Types::ExprChunkBits) {
    if (terms.empty())
        return store->makeFalse(bitWidth).id;

    if (terms.size() == 1)
        return terms[0];

    return store->create(Expression::OpType::Xor, std::move(terms), bitWidth).id;
}

// #pragma region MakeExpr
// inline ExprRef Make_Expr_Not(ExprStore* store, Ids::ExprId id) {
//     const Expr& e = store->get(id);
//     return store->create(OpType::Not, {id}, e.bitWidth);
// }
// #pragma endregion

inline int CompareExprCanonical(const Expression::ExprStore* store, Ids::ExprId a, Ids::ExprId b) {
    if (a == b)
        return 0;

    const Expression::Expr& exprA = store->get(a);
    const Expression::Expr& exprB = store->get(b);

    // --- op type ---
    if (exprA.op != exprB.op)
        return static_cast<int>((OpType)exprA.op) < static_cast<int>((OpType)exprB.op) ? -1 : 1;

    // --- variables ---
    if (exprA.op == Expression::OpType::Var)
        return a.value() < b.value() ? -1 : 1;

    // --- constants ---
    if (exprA.op == Expression::OpType::Const) {
        if (exprA.knownValue != exprB.knownValue)
            return exprA.knownValue < exprB.knownValue ? -1 : 1;

        return 0;
    }

    // --- arity ---
    if (exprA.inputs.size() != exprB.inputs.size())
        return exprA.inputs.size() < exprB.inputs.size() ? -1 : 1;

    // --- recursive compare ---
    for (size_t i = 0; i < exprA.inputs.size(); ++i) {
        const int inputCmp = CompareExprCanonical(store, exprA.inputs[i], exprB.inputs[i]);

        if (inputCmp != 0)
            return inputCmp;
    }

    return 0;
}

inline bool CanonicalExprLess(const ExprStore* store, Ids::ExprId a, Ids::ExprId b) {
    return CompareExprCanonical(store, a, b) < 0;
}

} // namespace BitFlow::Core::Expression