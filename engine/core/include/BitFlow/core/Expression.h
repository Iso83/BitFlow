#pragma once

#include <BitFlow/core/ids/ExprId.h>
#include <BitFlow/core/OpType.h>
#include <cstdint>
#include <vector>

namespace BitFlow::Core {

struct Expr {
    Ids::ExprId id{};

    OpType op{};
    std::vector<Expr*> inputs{};

    bool isConst{false};
    uint32_t constValue{0};
};

} // namespace BitFlow::Core