#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace BitFlow::Core::Expression {
struct ExprOld;
}

namespace BitFlow::Core::Codegen {

std::string EmitCRuntimeSupport(uint32_t bitWidth);
std::string EmitCExpr(const Expression::ExprOld* root, uint32_t bitWidth);
std::string EmitCFunction(const Expression::ExprOld* root, uint32_t bitWidth);
std::string EmitCFunctionMulti(const std::vector<const Expression::ExprOld*>& roots, uint32_t bitWidth);

} // namespace BitFlow::Core::Codegen
