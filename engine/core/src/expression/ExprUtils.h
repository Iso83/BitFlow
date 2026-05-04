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

#pragma region MakeExpr
inline ExprRef Make_Expr_Not(ExprStore* store, Ids::ExprId id) {
    const Expr& e = store->get(id);
    return store->create(OpType::Not, {id}, e.bitWidth);
}
#pragma endregion
} // namespace BitFlow::Core::Expression