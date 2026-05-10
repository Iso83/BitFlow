#pragma once

#include "TestAssert.h"

#include <BitFlow/core/rules/RuleEngine.h>
#include <iostream>

namespace BitFlow::Core::Testing {

#define BF_VALIDATE_ENGINE(engine, rule) BF_TEST(PrintDependencyValidation(engine.ValidateMinimalDependencies(rule)))

inline bool PrintDependencyValidation(const Rules::DependencyValidationResult& result) {

    if (result.valid)
        return true;

    std::cout << "\n=== Dependency Validation ===\n";
    std::cout << "Rule: " << result.rule.value << "\n";

    if (!result.missing.empty()) {
        std::cout << "\nMissing:\n";

        for (const auto& key : result.missing)
            std::cout << " - " << key.value << "\n";
    }

    if (!result.extra.empty()) {
        std::cout << "\nExtra:\n";

        for (const auto& key : result.extra)
            std::cout << " - " << key.value << "\n";
    }

    std::cout << std::endl;

    return false;
}

inline Expression::ExprRef Rewrite(Expression::ExprStore* store, Rules::RuleEngine& engine, Ids::ExprId id) {
    return Expression::ExprRef(store, engine.Rewrite(store, id));
}

inline Expression::ExprRef
Rewrite(Rules::RuleEngine& engine, Expression::ExprRef e,
        const std::unordered_map<BitFlow::Core::Ids::ExprId, std::string>* traceNames = nullptr,
        const Expression::PrintOptions& options = {}) {

    if (traceNames == nullptr)
        return Rewrite(e.store, engine, e.id);

    std::cout << "=== Rewrite Trace ===" << std::endl;
    std::cout << "Input : " << ToString(e.store, e.id, *traceNames, options) << std::endl;

    engine.SetDebugCallback(
        [traceNames, step = 0, e, options](Ids::ExprId before, Ids::ExprId after, Rules::RuleKey key) mutable {
            if (before == after)
                return;

            std::cout << "#" << step++ << " [" << key.value << "] " << ToString(e.store, before, *traceNames, options)
                      << " -> " << ToString(e.store, after, *traceNames, options) << std::endl;
        });

    auto result = Rewrite(e.store, engine, e.id);

    engine.SetDebugCallback(nullptr);

    std::cout << "Result: " << ToString(result.store, result.id, *traceNames, options) << std::endl;
    std::cout << std::endl;

    return result;
}

} // namespace BitFlow::Core::Testing