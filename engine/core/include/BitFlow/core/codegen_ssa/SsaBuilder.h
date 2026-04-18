#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <BitFlow/core/ast/OpType.h>

namespace BitFlow::Core::AST {
struct Expr;
}

namespace BitFlow::Core::Codegen {


struct Statement {
    uint32_t id;
    AST::OpType op;
    std::vector<uint32_t> inputs;
};

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

SsaProgram BuildSSA(const AST::Expr* root, uint32_t bitWidth);

} // namespace BitFlow::Core::Codegen
