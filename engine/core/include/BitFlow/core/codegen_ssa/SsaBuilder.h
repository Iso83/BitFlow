#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace BitFlow::Core::AST {
struct Expr;
}

namespace BitFlow::Core::Codegen {

struct SsaStatement {
    std::string name;
    std::string expr;
};

struct SsaProgram {
    std::vector<SsaStatement> statements;
    std::string result;
};

SsaProgram BuildSSA(const AST::Expr* root, uint32_t bitWidth);

} // namespace BitFlow::Core::Codegen
