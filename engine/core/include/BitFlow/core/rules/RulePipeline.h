#pragma once

#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>

namespace BitFlow::Core::Rules {

// =========================================================
// Normalize (global)
// =========================================================
inline void Add_Normalize_Rules(RuleEngine& engine) {
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
}

// =========================================================
// Simplify (Bitwise)
// =========================================================
inline void Add_Simplify_Bitwise_Rules(RuleEngine& engine) {

    // NOT
    engine.AddRule(Simplify::Bitwise::Get_NotPushdown_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Not_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Not_Xor_Rule());

    // Cancel
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Cancel_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Or_Cancel_Rule());

    // Reduction
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());

    // Fold
    engine.AddRule(Simplify::Bitwise::Get_Xor_Fold_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Fold_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Or_Fold_Rule());

    // Neutral
    engine.AddRule(Simplify::Bitwise::Get_Xor_Zero_Rule());

    // Structural
    engine.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Idempotent_Rule());

    // Logical
    engine.AddRule(Simplify::Bitwise::Get_Complement_Rule());

    // Dominance
    engine.AddRule(Simplify::Bitwise::Get_And_ZeroDominance_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_OneIdentity_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Or_OneDominance_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Or_ZeroIdentity_Rule());
}

// =========================================================
// Factorize (Bitwise)
// =========================================================
inline void Add_Factorize_Bitwise_Rules(RuleEngine& engine) {

    engine.AddRule(Factorize::Bitwise::Get_Xor_And_Rule());
    engine.AddRule(Factorize::Bitwise::Get_Xor_Pair_Cancel_Rule());

    engine.AddRule(Factorize::Bitwise::Get_And_Absorb_Rule());
    engine.AddRule(Factorize::Bitwise::Get_Or_Absorb_Rule());

    engine.AddRule(Factorize::Bitwise::Get_Distribute_Rule());
}

// =========================================================
// Simplify (Arithmetic)
// =========================================================
inline void Add_Simplify_Arithmetic_Rules(RuleEngine& engine) {
    engine.AddRule(Simplify::Arithmetic::Get_Add_Zero_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Sub_Zero_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Mul_One_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Mul_Zero_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Div_One_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Mod_Zero_Guard_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Neg_Neg_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Add_Fold_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Const_Combine_Rule());
}

// =========================================================
// Factorize (Arithmetic)
// =========================================================
inline void Add_Factorize_Arithmetic_Rules(RuleEngine& engine) {
    engine.AddRule(Factorize::Arithmetic::Get_Add_CommonFactor_Rule());
}

// =========================================================
// Simplify (SHA - optional)
// =========================================================
inline void Add_Simplify_SHA_Rules(RuleEngine& engine) {
    engine.AddRule(Simplify::Get_CH_Simplify_Rule());
    engine.AddRule(Simplify::Get_MAJ_Simplify_Rule());
}

} // namespace BitFlow::Core::Rules