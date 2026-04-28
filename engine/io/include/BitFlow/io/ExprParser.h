#pragma once

#include <BitFlow/core/expression/Expression.h>
#include <string>
#include <unordered_map>

namespace BitFlow::IO {

struct ParseResult {
    Core::Expression::ExprOld* root;
    std::unordered_map<uint32_t, std::string> idToName;
};

ParseResult Parse(const std::string& input);

} // namespace BitFlow::IO