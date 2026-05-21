#pragma once

#include "TestAssert.h"

#include <BitFlow/core/expression/ExprPrinter.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <iostream>

namespace BitFlow::Testing {

#define BF_VALIDATE_ENGINE(engine, rule) BF_TEST(PrintDependencyValidation(engine.AnalyzeDependencies(rule)))

inline bool PrintDependencyValidation(const Core::Rules::DependencyValidationResult& result) {

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

    if (!result.redundant.empty()) {
        std::cout << "\nRedundant:\n";

        for (const auto& key : result.redundant)
            std::cout << " - " << key.value << "\n";
    }

    std::cout << std::endl;

    return false;
}

inline Core::Expression::ExprRef Rewrite(Core::Expression::ExprStore* store, Core::Rules::RuleEngine& engine,
                                         Core::Ids::ExprId id) {
    return Core::Expression::ExprRef(store, engine.Rewrite(store, id));
}

inline Core::Expression::ExprRef Rewrite(Core::Rules::RuleEngine& engine, Core::Expression::ExprRef e,
                                         const Core::Expression::ExprNameMap* traceNames = nullptr,
                                         const Core::Expression::PrintOptions& options = {}) {

    if (traceNames == nullptr)
        return Rewrite(e.store, engine, e.id);

    std::cout << "=== Rewrite Trace ===" << std::endl;
    std::cout << "Input : " << ToString(e.store, e.id, *traceNames, options) << std::endl;

    engine.SetDebugCallback([traceNames, step = 0, e, options](Core::Ids::ExprId before, Core::Ids::ExprId after,
                                                               Core::Rules::RuleKey key) mutable {
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

#ifdef BF_TEST_CATCH_EXCEPTIONS
#define BF_SAFE_REWRITE(field, rewrite)                                                                                \
    BitFlow::Core::Expression::ExprRef field{};                                                                        \
    try {                                                                                                              \
        field = rewrite;                                                                                               \
    } catch (const std::exception& ex) {                                                                               \
        return -1;                                                                                                     \
    }
#else
#define BF_SAFE_REWRITE(field, rewrite)                                                                                \
    BitFlow::Core::Expression::ExprRef field{};                                                                        \
    field = rewrite;
#endif

} // namespace BitFlow::Testing