#pragma once

#include <BitFlow/core/Expression.h>
#include <vector>

namespace BitFlow::Core {

struct ExprKey {
    OpType op{};
    std::vector<uint32_t> inputs;
    bool isConst{false};
    uint32_t constValue{0};

    // voor symbolische leafs / variables
    bool hasSymbolId{false};
    uint32_t symbolId{0};

    bool operator==(const ExprKey& other) const {
        return op == other.op && inputs == other.inputs && isConst == other.isConst && constValue == other.constValue &&
               hasSymbolId == other.hasSymbolId && symbolId == other.symbolId;
    }
};

} // namespace BitFlow::Core