#pragma once

namespace BitFlow::Core {
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
} // namespace BitFlow::Core
