#pragma once

#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Codegen {

struct Statement;

void ApplyCSE(std::vector<Statement>& stmts);

} // namespace BitFlow::Core::Codegen
