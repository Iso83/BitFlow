#pragma once

#include "TestAssert.h"

#include <BitFlow/engine/core/rules/RulePipeline.h>
#include <BitFlow/engine/core/rules/RuleTrace.h>
#include <iostream>

namespace BitFlow::Testing {

#define BF_VALIDATE_ENGINE(engine, rule) CPPTEST_ASSERT(PrintDependencyValidation(engine.AnalyzeDependencies(rule)))

inline bool PrintDependencyValidation(const Engine::Core::Rules::DependencyValidationResult& result) {

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

#ifdef BitFlow_TEST_CATCH_EXCEPTIONS
#define BF_SAFE_REWRITE(field, rewrite)                                                                                \
    BitFlow::Engine::Core::Expression::ExprRef field{};                                                                \
    try {                                                                                                              \
        field = rewrite;                                                                                               \
    } catch (const std::exception& ex) {                                                                               \
        return -1;                                                                                                     \
    }
#else
#define BF_SAFE_REWRITE(field, rewrite)                                                                                \
    BitFlow::Engine::Core::Expression::ExprRef field{};                                                                \
    field = rewrite;
#endif

#define BF_REWRITE(expr, ...) Rewrite(engine, names, expr, __VA_ARGS__)

inline Engine::Core::Expression::ExprRef Rewrite(Engine::Core::Rules::RuleEngine& engine,
                                                 const Engine::Core::Expression::ExprNameMap& names,
                                                 Engine::Core::Expression::ExprRef e, const bool trace = false,
                                                 const Engine::Core::Expression::PrintOptions& options = {}) {

    if (!trace)
        return Engine::Core::Expression::ExprRef(e.store, engine.Rewrite(e.store, e.id, &names));

    std::cout << "=== Rewrite Trace ===" << std::endl;
    std::cout << "Input : " << ToString(e.store, e.id, names, options) << std::endl;

    std::cout << "Trace: " << std::endl;
    Engine::Core::Rules::AttachConsoleTrace(engine, names, options);
    std::cout << std::endl;

    auto result = Engine::Core::Expression::ExprRef(e.store, engine.Rewrite(e.store, e.id, &names));

    engine.SetDebugCallback(nullptr);

    std::cout << "Result: " << ToString(result.store, result.id, names, options) << std::endl;
    std::cout << std::endl;

    return result;
}
} // namespace BitFlow::Testing
