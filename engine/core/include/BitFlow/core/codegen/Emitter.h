#pragma once

#include <cstdint>
#include <string>

namespace BitFlow::Core::AST {
    struct Expr;
}

namespace BitFlow::Core::Codegen {

std::string EmitCExpr(const AST::Expr* root, uint32_t bitWidth);

} // namespace BitFlow::Core::Codegen
