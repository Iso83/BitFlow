#pragma once

#include <BitFlow/core/Expression.h>

namespace BitFlow::Core {

// commutative check
inline bool IsCommutative(OpType op) {
    switch (op) {
    case OpType::Add:
    case OpType::Xor:
    case OpType::And:
    case OpType::Or:
        return true;
    default:
        return false;
    }
}

// generic zero match
template <OpType Op> inline bool Match_Zero(const Expr& e) {
    if (e.op != Op)
        return false;

    if (e.inputs.empty())
        return false;

    for (const Expr* in : e.inputs) {
        if (in->isConst && in->constValue == 0)
            return true;
    }

    return false;
}

} // namespace BitFlow::Core