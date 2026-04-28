#pragma once

#include <cstdint>

namespace BitFlow::Core::Expression {
enum class ExprFlags : uint32_t {
    None = 0,
    KnownZero = 1u << 0,
    KnownOne = 1u << 1,
    KnownAllOnes = 1u << 2,
    // IsConstant = 1u << 3,
    HasKnownBits = 1u << 4,
    HasLargeConst = 1u << 5
};

inline ExprFlags operator|(ExprFlags a, ExprFlags b) {
    return static_cast<ExprFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline ExprFlags operator&(ExprFlags a, ExprFlags b) {
    return static_cast<ExprFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline ExprFlags& operator|=(ExprFlags& a, ExprFlags b) {
    a = a | b;
    return a;
}

inline bool HasFlag(ExprFlags flags, ExprFlags flag) {
    return (flags & flag) != ExprFlags::None;
}
} // namespace BitFlow::Core::Expression