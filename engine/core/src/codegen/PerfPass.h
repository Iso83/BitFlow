#pragma once

#include <cstdint>
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

// convenience
void ApplyPerfPass(std::vector<Statement>& stmts, uint32_t rootId);

} // namespace BitFlow::Core::Codegen
