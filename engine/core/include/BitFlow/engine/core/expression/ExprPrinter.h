#pragma once

#include <BitFlow/engine/core/expression/ExprStore.h>
#include <BitFlow/engine/core/expression/PrintOptions.h>

namespace BitFlow::Engine::Core::Expression {

std::string ToString(const ExprStore* store, const Ids::ExprId e);
std::string ToString(const ExprStore* store, const Ids::ExprId e, const ExprNameMap& names);
std::string ToString(const ExprStore* store, const Ids::ExprId e, const ExprNameMap& names,
                     const PrintOptions& options);

} // namespace BitFlow::Core::Expression
