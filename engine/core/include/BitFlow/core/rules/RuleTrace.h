#pragma once

#include <BitFlow/core/expression/ExprPrinter.h>
#include <BitFlow/core/rules/RuleEngine.h>

namespace BitFlow::Core::Rules {
void AttachConsoleTrace(RuleEngine& engine, const Expression::ExprNameMap& names = {},
                        const Expression::PrintOptions& printOptions = {});
}