#pragma once

namespace BitFlow::Core::AST {

enum class OpType {
    Add,
    Xor,
    And,
    Or,
    Not,
    // later:
    Ch,
    Maj,
    RotR,
};

} // namespace BitFlow::Core::AST
