#pragma once

#include <BitFlow/engine/core/ids/StrongId.h>
#include <string>
#include <unordered_map>

namespace BitFlow::Engine::Core::Ids {

struct ExprTag;
using ExprId = StrongId<ExprTag>;

} // namespace BitFlow::Core::Ids

namespace BitFlow::Engine::Core::Expression {

using ExprNameMap = std::unordered_map<Ids::ExprId, std::string>;

} // namespace BitFlow::Core::Expression
