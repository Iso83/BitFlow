#pragma once

#include <BitFlow/core/Rule.h>
#include <BitFlow/core/RuleEngine.h>

namespace BitFlow::Core {

inline void Add_Bitwise_Simplify_Pipeline(RuleEngine& engine) {
    engine.AddRule(Get_Flatten_Rule());
    engine.AddRule(Get_Order_Rule());

    engine.AddRule(Get_Xor_Cancel_Rule());
    engine.AddRule(Get_Xor_Fold_Rule());
    engine.AddRule(Get_Xor_Zero_Rule());

    engine.AddRule(Get_And_Fold_Rule());
    engine.AddRule(Get_Or_Fold_Rule());
}

} // namespace BitFlow::Core