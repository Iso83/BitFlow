#pragma once

#include <BitFlow/core/ids/StrongId.h>
#include <string>
#include <unordered_map>

namespace BitFlow::Core::Ids {

struct ExprTag;
using ExprId = StrongId<ExprTag>;

} // namespace BitFlow::Core::Ids

namespace BitFlow::Core::Expression {

using ExprNameMap = std::unordered_map<Ids::ExprId, std::string>;

} // namespace BitFlow::Core::Expression