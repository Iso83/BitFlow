#pragma once

#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>

namespace BitFlow::Core::Rules {

inline int CompareExprCanonical(const Expression::ExprOld* a, const Expression::ExprOld* b) {
    if (a == b)
        return 0;

    if (a == nullptr)
        return -1;

    if (b == nullptr)
        return 1;

    if (a->op != b->op)
        return static_cast<int>(a->op) < static_cast<int>(b->op) ? -1 : 1;

    if (a->constValue != b->constValue)
        return a->constValue < b->constValue ? -1 : 1;

    if (a->inputs.size() != b->inputs.size())
        return a->inputs.size() < b->inputs.size() ? -1 : 1;

    for (size_t i = 0; i < a->inputs.size(); ++i) {
        const int inputCmp = CompareExprCanonical(a->inputs[i], b->inputs[i]);
        if (inputCmp != 0)
            return inputCmp;
    }

    if (a->op == Expression::OpType::Var) {
        if (a->id.value() == b->id.value())
            return 0;
        return a->id.value() < b->id.value() ? -1 : 1;
    }

    return 0;
}

inline bool CanonicalExprLess(const Expression::ExprOld* a, const Expression::ExprOld* b) {
    return CompareExprCanonical(a, b) < 0;
}

template <Expression::OpType Op> inline bool Match_Zero(const Expression::ExprOld& e) {
    if (e.op != Op)
        return false;

    if (e.inputs.empty())
        return false;

    for (const Expression::ExprOld* in : e.inputs) {
        if (in->op == Expression::OpType::Const && in->constValue == 0)
            return true;
    }

    return false;
}

} // namespace BitFlow::Core::Rules
