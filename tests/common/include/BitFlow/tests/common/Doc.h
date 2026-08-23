#pragma once

#include "Expr.h"
#include "Rule.h"

#include <string>
#include <unordered_set>
#include <vector>

namespace BitFlow::Testing {

struct DocExample {
    Engine::Core::Rules::RuleKey rule;

    const char* input;
    const char* expected;
    std::vector<Engine::Core::Rules::RuleKey> disabledRules{};
    bool expand = false;
    bool trace = false;
};

bool ValidateTrace(const Engine::Core::Rules::RuleEngine& engine, Engine::Core::Rules::RuleKey target,
                   const std::unordered_set<Engine::Core::Rules::RuleKey>& usedRules) {
    auto allowed = engine.CollectRequiredRules(target);

    bool ok = true;

    for (const auto& used : usedRules) {

        if (allowed.contains(used))
            continue;

        std::cout << "\nUnexpected rule detected:\n"
                  << "  Target : " << target.value << "\n"
                  << "  Used   : " << used.value << "\n";

        ok = false;
    }

    if (!usedRules.contains(target)) {

        std::cout << "\nTarget rule never executed:\n"
                  << "  Rule : " << target.value << "\n";

        ok = false;
    }

    return ok;
}

int ValidateDocExample(Engine::Core::Rules::RuleEngine& engine, const DocExample& ex, const bool trace = false,
                       const Engine::Core::Expression::PrintOptions& options = {}) {
    Engine::Core::Expression::ExprStore store;

    auto parsed = Engine::IO::Parse(&store, ex.input);

    std::unordered_set<Engine::Core::Rules::RuleKey> usedRules;
    auto traceState = std::make_shared<std::string>();
    auto stepState = std::make_shared<int>(0);
    engine.SetDebugCallback([&](Engine::Core::Rules::RuleEngine::DebugCallBack_Ctx& ctx) {
        const auto key = ctx.key;
        usedRules.insert(key);

        if (trace) {
            ctx.beginCallback = [&, traceState](Engine::Core::Ids::ExprId before) {
                *traceState = ToString(ctx.store, before, parsed.names, options);
            };

            ctx.endCallback = [&, traceState, stepState, key](Engine::Core::Ids::ExprId after) {
                std::cout << "#" << (*stepState)++ << " [" << key.value << "]\n";
                std::cout << "    " << *traceState << "\n";
                std::cout << "    -> " << Engine::Core::Expression::ToString(ctx.store, after, parsed.names, options)
                          << "\n";
            };
        }
    });

    if (trace) {
        std::cout << "=== Rewrite Trace ===" << std::endl;
        std::cout << "Input : " << Engine::Core::Expression::ToString(&store, parsed.root.id, parsed.names, options)
                  << std::endl;

        std::cout << "Trace: " << std::endl;
    }

    BF_SAFE_REWRITE(rewritten,
                    Engine::Core::Expression::ExprRef(&store, engine.Rewrite(&store, parsed.root.id, &parsed.names)));

    auto result = Engine::Core::Expression::ToString(rewritten.store, rewritten.id, parsed.names);

    if (trace) {
        std::cout << std::endl;
        std::cout << "Result: " << result << std::endl;
        std::cout << std::endl;
    }

    CPPTEST_ASSERT(result == ex.expected);
    CPPTEST_ASSERT(ValidateTrace(engine, ex.rule, usedRules));
    return 0;
}

} // namespace BitFlow::Testing
