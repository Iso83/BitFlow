#pragma once

#include <BitFlow/engine/core/expression/ExprPrinter.h>
#include <BitFlow/engine/core/rules/RuleEngine.h>

namespace BitFlow::Engine::Core::Rules {
void AttachConsoleTrace(RuleEngine& engine, const Expression::ExprNameMap& names = {},
                        const Expression::PrintOptions& printOptions = {});
}
