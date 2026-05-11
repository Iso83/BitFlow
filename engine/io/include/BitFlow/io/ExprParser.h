#pragma once

#include <BitFlow/io/ExprPrinter.h>
#include <string>

namespace BitFlow::IO {

struct ParseResult {
    Core::Expression::ExprRef root;
    ExprNameMap names;
};

ParseResult Parse(Core::Expression::ExprStore* store, const std::string& input);

} // namespace BitFlow::IO