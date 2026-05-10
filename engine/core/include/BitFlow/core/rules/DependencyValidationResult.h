#pragma once

#include <BitFlow/core/rules/RuleKey.h>
#include <vector>

namespace BitFlow::Core::Rules {

struct DependencyValidationResult {
    RuleKey rule;

    bool valid = true;

    std::vector<RuleKey> missing;
    std::vector<RuleKey> extra;

    explicit DependencyValidationResult(RuleKey r) : rule(r) {}
};

} // namespace BitFlow::Core::Rules