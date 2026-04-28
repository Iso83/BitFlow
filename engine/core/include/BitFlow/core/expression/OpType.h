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

}