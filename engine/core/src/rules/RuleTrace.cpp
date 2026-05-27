#include <BitFlow/core/rules/RuleTrace.h>
#include <iostream>

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

namespace BitFlow::Core::Rules {
void AttachConsoleTrace(RuleEngine& engine, const ExprNameMap& names, const PrintOptions& printOptions) {
    auto traceState = std::make_shared<std::string>();
    auto stepState = std::make_shared<int>(0);

    engine.SetDebugCallback([&, traceState, stepState](RuleEngine::DebugCallBack_Ctx& debugCtx) {
        const auto key = debugCtx.key;

        debugCtx.beginCallback = [&, traceState](ExprId before) {
            *traceState = ToString(debugCtx.store, before, names, printOptions);
        };

        debugCtx.endCallback = [&, traceState, stepState, key](ExprId after) {
            std::cout << "#" << (*stepState)++ << " [" << key.value << "]\n";
            std::cout << "    " << *traceState << "\n";
            std::cout << "    -> " << ToString(debugCtx.store, after, names, printOptions) << "\n";
        };
    });
}
} // namespace BitFlow::Core::Rules