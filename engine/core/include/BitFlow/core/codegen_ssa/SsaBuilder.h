#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace BitFlow::Core::Expression {
struct ExprOld;
}

namespace BitFlow::Core::Codegen {

struct SsaStatement {
    std::string name;
    std::string expr;
};

struct SsaProgram {
    std::vector<SsaStatement> statements;
    std::string result;

    // Step 14.7 (later): reserved for multi-output SSA construction.
    // Current BuildSSA(root, bitWidth) fills only `result`.
    std::vector<std::string> results;
};

SsaProgram BuildSSA(const Expression::ExprOld* root, uint32_t bitWidth);
SsaProgram BuildSSA(const std::vector<const Expression::ExprOld*>& roots, uint32_t bitWidth);

} // namespace BitFlow::Core::Codegen
