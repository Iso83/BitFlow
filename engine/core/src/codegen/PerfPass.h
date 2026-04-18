#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace BitFlow::Core::Codegen {

struct Statement {
    uint32_t id;
    uint32_t op;
    std::vector<uint32_t> inputs;
};

void ApplyCSE(std::vector<Statement>& stmts);
void ApplyDCE(std::vector<Statement>& stmts, uint32_t rootId);
void ApplyTempReuse(std::vector<Statement>& stmts);
void ApplyConstantFolding(std::vector<Statement>& stmts, uint32_t bitWidth);

// convenience
void ApplyPerfPass(std::vector<Statement>& stmts, uint32_t rootId);
void ApplyPerfPass(std::vector<Statement>& stmts, uint32_t& rootId,
                   std::unordered_map<uint32_t, uint64_t>& constValues, uint32_t bitWidth);

} // namespace BitFlow::Core::Codegen
