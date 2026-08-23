#pragma once

#include <BitFlow/engine/core/expression/ExprPrinter.h>
#include <BitFlow/engine/core/expression/ExprRef.h>
#include <BitFlow/engine/core/expression/ExprStore.h>
#include <string>

namespace BitFlow::Engine::IO {

[[nodiscard]]
std::string ToLatex(const BitFlow::Engine::Core::Expression::ExprStore* store, BitFlow::Engine::Core::Ids::ExprId root,
                    const BitFlow::Engine::Core::Expression::ExprNameMap& names = {});

[[nodiscard]]
std::string ToLatex(BitFlow::Engine::Core::Expression::ExprRef root,
                    const BitFlow::Engine::Core::Expression::ExprNameMap& names = {});

} // namespace BitFlow::Engine::IO
