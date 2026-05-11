#pragma once

#include <BitFlow/core/expression/ExprPrinter.h>
#include <BitFlow/core/expression/ExprRef.h>
#include <BitFlow/core/expression/ExprStore.h>
#include <string>

namespace BitFlow::IO {

[[nodiscard]]
std::string ToLatex(const BitFlow::Core::Expression::ExprStore* store, BitFlow::Core::Ids::ExprId root,
                    const BitFlow::Core::Expression::ExprNameMap& names = {});

[[nodiscard]]
std::string ToLatex(BitFlow::Core::Expression::ExprRef root, const BitFlow::Core::Expression::ExprNameMap& names = {});

} // namespace BitFlow::IO