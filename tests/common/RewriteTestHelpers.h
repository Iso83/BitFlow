#pragma once

#include <BitFlow/core/rules/RuleEngine.h>

namespace BitFlow::Tests {

inline bool RewriteStable(const BitFlow::Core::Rules::RewriteResult& info) {
    return info.stable;
}

inline bool RewriteHasNoCycle(const BitFlow::Core::Rules::RewriteResult& info) {
    return !info.cycleDetected();
}

inline bool RewriteWithinIterationLimit(const BitFlow::Core::Rules::RewriteResult& info, int limit) {
    return info.IterationsWithin(limit);
}

inline bool RewriteStableWithoutCycleWithin(const BitFlow::Core::Rules::RewriteResult& info, int limit) {
    return info.StableWithoutCycle() && info.IterationsWithin(limit);
}

} // namespace BitFlow::Tests
