#pragma once

namespace BitFlow::Core::Rules {

enum RuleStage {
    Stage_Normalize = 0, // flatten/order

    // Simplify - zero/fold/cancel
    Stage_Simplify_Pushdown = 1,
    Stage_Simplify = 2,

    Stage_Factorize = 3 // patterns
};

} // namespace BitFlow::Core::Rules