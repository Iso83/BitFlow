#pragma once

#include <BitFlow/core/ast/OpType.h>
#include <cstdint>
#include <vector>

namespace BitFlow::Core::Expression {

struct ExprKey {
    AST::OpType op{};
    std::vector<uint32_t> inputs;
    bool isConst{false};
    uint32_t constValue{0};

    bool operator==(const ExprKey& other) const {
        return op == other.op && inputs == other.inputs && isConst == other.isConst && constValue == other.constValue;
    }
};

} // namespace BitFlow::Core::Expression
