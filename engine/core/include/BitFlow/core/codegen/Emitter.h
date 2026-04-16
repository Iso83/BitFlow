#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace BitFlow::Core::AST {
struct Expr;
}

namespace BitFlow::Core::Codegen {

std::string EmitCExpr(const AST::Expr* root, uint32_t bitWidth);
std::string EmitCFunction(const AST::Expr* root, uint32_t bitWidth);
std::map<uint32_t, std::string> BuildVarNameMap(const AST::Expr* root,
                                                const std::map<uint32_t, std::string>& overrides = {});
std::string EmitCParamList(const AST::Expr* root, uint32_t bitWidth,
                           const std::map<uint32_t, std::string>& varNames = {});
std::string EmitCFunction(const AST::Expr* root, uint32_t bitWidth, const std::string& functionName,
                          const std::map<uint32_t, std::string>& varNames = {});

} // namespace BitFlow::Core::Codegen
