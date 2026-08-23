#pragma once

#include <BitFlow/engine/core/expression/ExprRef.h>
#include <BitFlow/engine/core/expression/ExprStore.h>
#include <span>
#include <string_view>

namespace BitFlow::Engine::IO {

struct FunctionResolveContext {
    Core::Expression::ExprStore* store{};
    std::string_view name{};
    std::span<const Core::Expression::ExprRef> args{};
};

class IFunctionResolver {
  public:
    virtual ~IFunctionResolver() = default;

    [[nodiscard]]
    virtual bool Contains(std::string_view name) const = 0;

    [[nodiscard]]
    virtual Core::Expression::ExprRef Resolve(FunctionResolveContext ctx) = 0;
};

} // namespace BitFlow::IO
