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
    UShl,

    // --- Bit ops ---
    RotL,
    RotR,

    // --- SHA / higher-level ---
    Ch,
    Maj,
};

} // namespace BitFlow::Core::AST
