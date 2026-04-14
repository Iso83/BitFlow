#pragma once

#include <BitFlow/core/ast/OpType.h>
#include <cstdint>
#include <vector>

namespace BitFlow::Core::Expression {

struct ExprKey {
    AST::OpType op{};
    std::vector<uint32_t> inputs;
    uint32_t constValue{0};

    bool operator==(const ExprKey& other) const {
        return op == other.op && inputs == other.inputs && constValue == other.constValue;
    }
};

} // namespace BitFlow::Core::Expression
