#pragma once

#include <BitFlow/core/expression/OpType.h>

namespace BitFlow::Core::Expression {

/*
Invariant reference per op
==========================

Leaf
- Var:   arity=0, fixed=true,  commutative=false, associative=false
- Const: arity=0, fixed=true,  commutative=false, associative=false

Unary
- Not: arity=1, fixed=true, commutative=false, associative=false
- Neg: arity=1, fixed=true, commutative=false, associative=false

Bitwise / n-ary
- And: arity=var, fixed=false, commutative=true,  associative=true
- Or:  arity=var, fixed=false, commutative=true,  associative=true
- Xor: arity=var, fixed=false, commutative=true,  associative=true

Arithmetic
- Add: arity=var, fixed=false, commutative=true,  associative=true
- Sub: arity=2,   fixed=true,  commutative=false, associative=false
- Mul: arity=var, fixed=false, commutative=true,  associative=true
- Div: arity=2,   fixed=true,  commutative=false, associative=false
- Mod: arity=2,   fixed=true,  commutative=false, associative=false

Shifts
- Shl:  arity=2, fixed=true, commutative=false, associative=false
- Shr:  arity=2, fixed=true, commutative=false, associative=false
- UShr: arity=2, fixed=true, commutative=false, associative=false

Rotations
- RotL: arity=2, fixed=true, commutative=false, associative=false
- RotR: arity=2, fixed=true, commutative=false, associative=false

SHA higher-level
- Ch:  arity=3, fixed=true, commutative=false, associative=false
- Maj: arity=3, fixed=true, commutative=false, associative=false
*/
inline constexpr bool IsLeaf(OpType op) {
    return op == OpType::Var || op == OpType::Const;
}

inline constexpr bool IsCommutative(OpType op) {
    switch (op) {
    case OpType::Add:
    case OpType::Mul:
    case OpType::And:
    case OpType::Or:
    case OpType::Xor:
        return true;
    default:
        return false;
    }
}

inline constexpr bool IsAssociative(OpType op) {
    switch (op) {
    case OpType::Add:
    case OpType::Mul:
    case OpType::And:
    case OpType::Or:
    case OpType::Xor:
        return true;
    default:
        return false;
    }
}

inline constexpr int ArityOf(OpType op) {
    switch (op) {
    case OpType::Var:
    case OpType::Const:
        return 0;
    case OpType::Not:
    case OpType::Neg:
        return 1;
    case OpType::Sub:
    case OpType::Div:
    case OpType::Mod:
    case OpType::Shl:
    case OpType::Shr:
    case OpType::UShr:
    case OpType::RotL:
    case OpType::RotR:
        return 2;
    case OpType::Ch:
    case OpType::Maj:
        return 3;
    default:
        return -1;
    }
}

inline constexpr bool HasFixedArity(OpType op) {
    return ArityOf(op) >= 0;
}

inline constexpr bool IsNestedSame(OpType parent, OpType child) {
    return !IsLeaf(child) && parent == child;
}

} // namespace BitFlow::Core::AST