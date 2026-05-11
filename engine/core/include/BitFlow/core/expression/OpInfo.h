#pragma once

#include <BitFlow/core/expression/OpType.h>
#include <cstdint>

namespace BitFlow::Core::Expression {

enum class Associativity { Left, Right, None };

struct OpInfo {
    uint8_t precedence{0};

    Associativity associativity{Associativity::None};

    const char* symbol{};

    bool infix{false};
    bool prefix{false};
};

[[nodiscard]]
const OpInfo* GetOpInfo(OpType op);

[[nodiscard]]
bool RequiresParentheses(OpType parent, OpType child, bool isRightChild);

} // namespace BitFlow::Core::Expression