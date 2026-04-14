#pragma once

#include <BitFlow/core/ast/Expression.h>
#include <cctype>
#include <string>
#include <unordered_map>

namespace BitFlow::IO {

struct PrintOptions {
    bool rotAsFunction = true;
};

std::string ToString(const Core::AST::Expr* e);
std::string ToString(const Core::AST::Expr* e, const std::unordered_map<uint32_t, std::string>& names);
std::string ToString(const Core::AST::Expr* e, const std::unordered_map<uint32_t, std::string>& names,
                     const PrintOptions& options);

} // namespace BitFlow::IO
