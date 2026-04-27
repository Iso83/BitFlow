#pragma once

#include <BitFlow/core/expression/OpType.h>
#include <cstdint>
#include <vector>

namespace BitFlow::Core::Expression {

struct ExprKey {
    OpType op{};
    std::vector<uint32_t> inputs;
    uint32_t constValue{0};

    bool operator==(const ExprKey& other) const {
        return op == other.op && inputs == other.inputs && constValue == other.constValue;
    }
};

} // namespace BitFlow::Core::Expression
