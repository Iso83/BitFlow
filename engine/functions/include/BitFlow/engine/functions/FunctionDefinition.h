#pragma once

#include <BitFlow/engine/core/expression/ExprRef.h>
#include <BitFlow/engine/core/expression/ExprStore.h>
#include <span>
#include <string_view>

namespace BitFlow::Engine::Functions {

struct FunctionExpandContext {
    Core::Expression::ExprStore* store{};
    std::span<Core::Ids::ExprId> args{};
};

using ExpandFn = Core::Expression::ExprRef (*)(FunctionExpandContext& ctx);

struct FunctionDefinition {
    std::string_view name;
    uint32_t parameterCount{};
    ExpandFn expand{};
};

} // namespace BitFlow::Functions
