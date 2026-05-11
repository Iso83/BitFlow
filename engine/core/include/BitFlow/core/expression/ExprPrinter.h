#pragma once

#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/expression/PrintOptions.h>
#include <string>
#include <unordered_map>

namespace BitFlow::Core::Expression {

using ExprNameMap = std::unordered_map<Ids::ExprId, std::string>;

std::string ToString(const ExprStore* store, const Ids::ExprId e);
std::string ToString(const ExprStore* store, const Ids::ExprId e, const ExprNameMap& names);
std::string ToString(const ExprStore* store, const Ids::ExprId e, const ExprNameMap& names,
                     const PrintOptions& options);

} // namespace BitFlow::Core::Expression