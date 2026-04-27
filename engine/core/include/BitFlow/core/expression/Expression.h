#pragma once

#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/ids/ExprId.h>
#include <cstdint>
#include <vector>

namespace BitFlow::Core::Expression {

struct Expr {
    Ids::ExprId id{};
    OpType op{};
    std::vector<Expr*> inputs{};

    // TODO: not here!
    uint32_t constValue{0};
};

} // namespace BitFlow::Core::AST
