#pragma once

#include <BitFlow/core/rules/RulePipeline.h>

namespace BitFlow::Core::Testing {

inline Rules::RuleEngine MakeShaSafeEngine() {
    return Rules::BuildProfile("sha_safe");
}

inline Rules::RuleEngine MakeFactorizeArithmeticSafeEngine() {
    return Rules::BuildProfile("factorize_arithmetic_safe");
}

inline Rules::RuleEngine MakeExploreEngine() {
    return Rules::BuildProfile("explore");
}

} // namespace BitFlow::Core::Testing
