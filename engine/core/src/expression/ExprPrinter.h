#pragma once

#include <BitFlow/core/expression/ExprStore.h>
#include <string>
#include <unordered_map>

namespace BitFlow::Core::Expression {

struct PrintOptions {
    bool rotAsFunction = true;
};

std::string ToString(const ExprStore* store, const Ids::ExprId e);
std::string ToString(const ExprStore* store, const Ids::ExprId e,
                     const std::unordered_map<Ids::ExprId, std::string>& names);
std::string ToString(const ExprStore* store, const Ids::ExprId e,
                     const std::unordered_map<Ids::ExprId, std::string>& names, const PrintOptions& options);

} // namespace BitFlow::Core::Expression