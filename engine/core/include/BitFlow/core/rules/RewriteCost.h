#pragma once

#include <cstdint>

namespace BitFlow::Core::Expression {
struct ExprOld;
}

namespace BitFlow::Core::Rules {

struct RewriteCost {
    uint32_t totalNodes{0};
    uint32_t operatorNodes{0};
    uint32_t maxDepth{0};
    uint32_t distributivePatterns{0};
    uint32_t nestedAssociativeNodes{0};
};

enum class RewriteCostPolicy {
    FactorizeSafe,
    ExpandDistribute,
};

RewriteCost ComputeRewriteCost(const Expression::ExprOld* expr);
bool IsRewritePreferred(const Expression::ExprOld* candidate, const Expression::ExprOld* current,
                        RewriteCostPolicy policy);

} // namespace BitFlow::Core::Rules
