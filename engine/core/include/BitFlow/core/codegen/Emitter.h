#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace BitFlow::Core::AST {
struct Expr;
}

namespace BitFlow::Core::Codegen {

std::string EmitCExpr(const AST::Expr* root, uint32_t bitWidth);
std::string EmitCFunction(const AST::Expr* root, uint32_t bitWidth);
std::string EmitCFunctionMulti(const std::vector<const AST::Expr*>& roots, uint32_t bitWidth);

} // namespace BitFlow::Core::Codegen
