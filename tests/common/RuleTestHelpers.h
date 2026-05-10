#pragma once

#include "TestAssert.h"

#include <BitFlow/core/rules/RuleEngine.h>
#include <iostream>

namespace BitFlow::Core::Testing {

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

#define BF_VALIDATE_ENGINE(engine, rule) BF_TEST(PrintDependencyValidation(engine.ValidateMinimalDependencies(rule)))

} // namespace BitFlow::Core::Testing