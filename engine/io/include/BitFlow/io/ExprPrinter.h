#pragma once

#include <BitFlow/core/ast/Expression.h>
#include <cctype>
#include <string>
#include <unordered_map>

namespace BitFlow::IO {

std::string ToString(const Core::AST::Expr* e);
std::string ToString(const Core::AST::Expr* e, const std::unordered_map<uint32_t, std::string>& names);

} // namespace BitFlow::IO