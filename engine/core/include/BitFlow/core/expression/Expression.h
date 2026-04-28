#pragma once
#include <BitFlow/core/expression/ExprFlags.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/helper/Attributes.h>
#include <BitFlow/core/ids/ExprId.h>
#include <vector>

namespace BitFlow::Core::Expression {

// TODO: Cleanup ExprOld
BF_DEPRECATED("Use Expr")
struct ExprOld {
    Ids::ExprId id{};
    OpType op{};
    std::vector<ExprOld*> inputs{};

    // TODO: not here!
    uint32_t constValue{0};

    /*private:
    ExprOld() = default;*/
};

struct Expr {
    Ids::ExprId id{};
    OpType op{};
    uint32_t generation{0};

    uint16_t bitWidth{0};
    uint16_t arity{0};

    ExprFlags flags{ExprFlags::None};

    uint64_t valueMask{0};
    uint64_t knownValue{0};
    uint64_t constValue{0};

    uint32_t largeConstIndex{0};

    std::vector<Ids::ExprId> inputs{};
};

} // namespace BitFlow::Core::Expression
