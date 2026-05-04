#pragma once

#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/ids/ExprId.h>
#include <vector>

namespace BitFlow::Core::Expression {

struct Expr {
    OpType op{};
    std::vector<Ids::ExprId> inputs{};

    uint16_t bitWidth{0};

    uint64_t knownMask{0};
    uint64_t knownValue{0};

    uint32_t largeConstIndex{0};

  public:
    [[nodiscard]] static uint64_t fullMask(uint16_t bitWidth) {
        if (bitWidth == 0)
            return 0;

        if (bitWidth >= 64)
            return ~uint64_t{0};

        return (uint64_t{1} << bitWidth) - 1;
    }
};

} // namespace BitFlow::Core::Expression
