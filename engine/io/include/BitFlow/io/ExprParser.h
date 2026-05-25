#pragma once

#include <BitFlow/io/ExprPrinter.h>
#include <BitFlow/io/IFunctionResolver.h>

namespace BitFlow::IO {

struct ParseResult {
    Core::Expression::ExprRef root;
    ExprNameMap names;
};

ParseResult Parse(Core::Expression::ExprStore* store, const std::string& input, IFunctionResolver* functions = nullptr);

} // namespace BitFlow::IO