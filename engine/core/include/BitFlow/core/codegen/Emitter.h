#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace BitFlow::Core::Expression {
struct Expr;
}

namespace BitFlow::Core::Codegen {

std::string EmitCRuntimeSupport(uint32_t bitWidth);
std::string EmitCExpr(const Expression::Expr* root, uint32_t bitWidth);
std::string EmitCFunction(const Expression::Expr* root, uint32_t bitWidth);
std::string EmitCFunctionMulti(const std::vector<const Expression::Expr*>& roots, uint32_t bitWidth);

} // namespace BitFlow::Core::Codegen
