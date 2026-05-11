#pragma once

#include <BitFlow/core/expression/ExprPrinter.h>

namespace BitFlow::IO {

using PrintOptions = Core::Expression::PrintOptions;
using ExprNameMap = Core::Expression::ExprNameMap;

inline std::string ToString(const Core::Expression::ExprRef& e) {
    return Core::Expression::ToString(e.store, e.id);
}

inline std::string ToString(const Core::Expression::ExprRef& e, const ExprNameMap& names) {
    return Core::Expression::ToString(e.store, e.id, names);
}

inline std::string ToString(const Core::Expression::ExprRef& e, const ExprNameMap& names, const PrintOptions& options) {
    return Core::Expression::ToString(e.store, e.id, names, options);
}

} // namespace BitFlow::IO