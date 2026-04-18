#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Codegen {

struct Statement;

void ApplyCSE(std::vector<Statement>& stmts);
void ApplyDCE(std::vector<Statement>& stmts, uint32_t rootId);

} // namespace BitFlow::Core::Codegen
