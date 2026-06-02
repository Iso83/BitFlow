#pragma once

#include "TestAssert.h"

#include <BitFlow/core/expression/ExprPrinter.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <BitFlow/core/rules/RuleTrace.h>
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

inline Core::Expression::ExprRef Rewrite(Core::Rules::RuleEngine& engine, const Core::Expression::ExprNameMap& names,
                                         Core::Expression::ExprRef e, const bool trace = false,
                                         const Core::Expression::PrintOptions& options = {}) {

    if (!trace)
        return Core::Expression::ExprRef(e.store, engine.Rewrite(e.store, e.id, &names));

    std::cout << "=== Rewrite Trace ===" << std::endl;
    std::cout << "Input : " << ToString(e.store, e.id, names, options) << std::endl;

    std::cout << "Trace: " << std::endl;
    Core::Rules::AttachConsoleTrace(engine, names, options);
    std::cout << std::endl;

    auto result = Core::Expression::ExprRef(e.store, engine.Rewrite(e.store, e.id, &names));

    engine.SetDebugCallback(nullptr);

    std::cout << "Result: " << ToString(result.store, result.id, names, options) << std::endl;
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

#define BF_REWRITE(expr, ...) Rewrite(engine, names, expr, __VA_ARGS__)

} // namespace BitFlow::Testing