#pragma once

namespace BitFlow::Core::AST {

enum class OpType {
    // --- Leaf ---
    Var,
    Const,

    // --- Unary ---
    Not,

    // --- Bitwise ---
    And,
    Or,
    Xor,

    // --- Arithmetic ---
    Add,

    // --- Bit ops ---
    RotR,

    // --- SHA / higher-level ---
    Ch,
    Maj,
};

} // namespace BitFlow::Core::AST
