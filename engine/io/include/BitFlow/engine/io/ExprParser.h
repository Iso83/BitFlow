#pragma once

#include <BitFlow/engine/io/ExprPrinter.h>
#include <BitFlow/engine/io/IFunctionResolver.h>

namespace BitFlow::Engine::IO {

struct ParseResult {
    Core::Expression::ExprRef root;
    ExprNameMap names;
};

ParseResult Parse(Core::Expression::ExprStore* store, const std::string& input, IFunctionResolver* functions = nullptr);

} // namespace BitFlow::IO
