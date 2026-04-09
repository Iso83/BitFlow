#pragma once

#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>

namespace BitFlow::Core::Rules {

inline void Add_Bitwise_Simplify_Pipeline(RuleEngine& engine) {
    engine.AddRule(Normalize::Get_Normalize_Flatten_Rule());
    engine.AddRule(Normalize::Get_Normalize_Order_Rule());

    engine.AddRule(Factorize::Get_Factorize_Xor_And_Rule());
    engine.AddRule(Factorize::Get_Factorize_Xor_Pair_Cancel_Rule());

    engine.AddRule(Simplify::Get_Simplify_Xor_Cancel_Rule());
    engine.AddRule(Simplify::Get_Simplify_Xor_Fold_Rule());
    engine.AddRule(Simplify::Get_Simplify_Xor_Zero_Rule());

    engine.AddRule(Simplify::Get_Simplify_And_Fold_Rule());
    engine.AddRule(Simplify::Get_Simplify_Or_Fold_Rule());
}

} // namespace BitFlow::Core::Rules