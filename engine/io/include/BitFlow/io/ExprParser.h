#pragma once

#include <BitFlow/core/ast/Expression.h>
#include <string>
#include <unordered_map>

namespace BitFlow::IO {

struct ParseResult {
    Core::AST::Expr* root;
    std::unordered_map<uint32_t, std::string> idToName;
};

ParseResult Parse(const std::string& input);

} // namespace BitFlow::IO