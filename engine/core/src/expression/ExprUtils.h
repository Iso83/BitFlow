#pragma once

#include <BitFlow/core/expression/ExprStore.h>

namespace BitFlow::Core::Expression {

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

inline bool IsFalse(const ExprStore* store, Ids::ExprId id) {
    const Expr& e = store->get(id);

    _ASSERT(e.op == OpType::Const && e.inputs.empty());

    return e.knownValue == 0;
}

inline bool IsTrue(const ExprStore* store, Ids::ExprId id) {
    const Expr& e = store->get(id);

    _ASSERT(e.op == OpType::Const && e.inputs.empty());

    return e.knownValue == Expr::fullMask(e.bitWidth);
}

inline Ids::ExprId MakeXor(Expression::ExprStore* store, std::vector<Ids::ExprId>& terms, uint16_t bitWidth = 64) {
    if (terms.empty())
        return store->makeFalse(bitWidth).id;

    if (terms.size() == 1)
        return terms[0];

    return store->create(Expression::OpType::Xor, std::move(terms), bitWidth).id;
}

#pragma region MakeExpr
inline ExprRef Make_Expr_Not(ExprStore* store, Ids::ExprId id) {
    const Expr& e = store->get(id);
    return store->create(OpType::Not, {id}, e.bitWidth);
}
#pragma endregion

BF_DEPRECATED("review: rommelige vorm")
inline int CompareExprCanonical(const Expression::ExprStore* store, Ids::ExprId a, Ids::ExprId b) {
    if (a == b)
        return 0;

    const Expression::Expr& exprA = store->get(a);
    const Expression::Expr& exprB = store->get(b);

    if (exprA.op != exprB.op)
        return static_cast<int>(exprA.op) < static_cast<int>(exprB.op) ? -1 : 1;

    if (exprA.knownValue != exprB.knownValue)
        return exprA.knownValue < exprB.knownValue ? -1 : 1;

    if (exprA.inputs.size() != exprB.inputs.size())
        return exprA.inputs.size() < exprB.inputs.size() ? -1 : 1;

    for (size_t i = 0; i < exprA.inputs.size(); ++i) {
        const int inputCmp = CompareExprCanonical(store, exprA.inputs[i], exprB.inputs[i]);
        if (inputCmp != 0)
            return inputCmp;
    }

    if (exprA.op == Expression::OpType::Var) {
        return a.value() < b.value() ? -1 : 1;
    }

    return 0;
}

inline bool CanonicalExprLess(const Expression::ExprStore* store, Ids::ExprId a, Ids::ExprId b) {
    return CompareExprCanonical(store, a, b) < 0;
}

template <Expression::OpType Op> inline bool Match_Zero(const Expression::ExprStore* store, Ids::ExprId id) {
    const Expression::Expr& e = store->get(id);
    if (e.op != Op)
        return false;

    if (e.inputs.empty())
        return false;

    for (auto in : e.inputs) {
        const Expression::Expr& exprIn = store->get(in);
        if (exprIn.op == Expression::OpType::Const && exprIn.inputs.empty() && exprIn.knownValue == 0)
            return true;
    }

    return false;
}

} // namespace BitFlow::Core::Expression