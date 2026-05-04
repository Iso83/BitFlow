#pragma once

#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/helper/Attributes.h>

namespace BitFlow::Core::Rules {

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

// template <Expression::OpType Op> inline bool Match_Zero(const Expression::ExprOld& e) {
//     if (e.op != Op)
//         return false;
//
//     if (e.inputs.empty())
//         return false;
//
//     for (const Expression::ExprOld* in : e.inputs) {
//         if (in->op == Expression::OpType::Const && in->constValue == 0)
//             return true;
//     }
//
//     return false;
// }

} // namespace BitFlow::Core::Rules
