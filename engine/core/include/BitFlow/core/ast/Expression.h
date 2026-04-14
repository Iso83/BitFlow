#pragma once

#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/ids/ExprId.h>
#include <cstdint>
#include <vector>

namespace BitFlow::Core::AST {

struct Expr {
    Ids::ExprId id{};
    OpType op{};
    std::vector<Expr*> inputs{};
    uint32_t constValue{0};
    bool frozen = false;

    bool isConst() const {
        return op == OpType::Const;
    }
};

} // namespace BitFlow::Core::AST
