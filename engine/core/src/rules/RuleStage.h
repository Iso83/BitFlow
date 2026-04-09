#pragma once

namespace BitFlow::Core::Rules {

enum RuleStage {
    Stage_Normalize = 0, // flatten/order
    Stage_Simplify = 1,  // zero/fold/cancel
    Stage_Factorize = 2  // patterns
};

} // namespace BitFlow::Core::Rules