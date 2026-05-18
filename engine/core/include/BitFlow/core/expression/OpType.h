#pragma once

namespace BitFlow::Core::Expression {

enum class OpType {
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
    Pow,

    // --- Shifts ---
    Shl,
    Shr,

    // --- Bit ops ---
    RotL,
    RotR,
};

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

} // namespace BitFlow::Core::Expression