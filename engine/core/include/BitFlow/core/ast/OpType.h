#pragma once

namespace BitFlow::Core::AST {

enum class OpType {
    // --- Leaf ---
    Var,
    Const,

    // --- Unary ---
    Not,
    Neg,

    // --- Bitwise ---
    And,
    Or,
    Xor,

    // --- Arithmetic ---
    Add,
    Sub,
    Mul,
    Div,
    Mod,

    // --- Shifts ---
    Shl,
    Shr,
    UShr,

    // --- Bit ops ---
    RotL,
    RotR,

    // --- SHA / higher-level ---
    Ch,
    Maj,
};

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

} // namespace BitFlow::Core::AST
