#pragma once

#include <BitFlow/engine/core/rules/RuleKey.h>
#include <vector>

namespace BitFlow::Engine::Core::Rules {

struct DependencyValidationResult {
    RuleKey rule;

    bool valid = true;

    std::vector<RuleKey> missing;
    std::vector<RuleKey> extra;
    std::vector<RuleKey> redundant;

    explicit DependencyValidationResult(RuleKey r) : rule(r) {}
};

} // namespace BitFlow::Core::Rules
