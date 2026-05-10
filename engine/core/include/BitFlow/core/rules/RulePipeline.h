#pragma once

#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>

namespace BitFlow::Core::Rules {

// =========================================================
// Normalize
// =========================================================
inline RuleEngine BuildNormalize() {
    RuleEngine e;

    e.AddRule(Normalize::Get_Flatten_Rule());
    e.AddRule(Normalize::Get_Order_Rule());
    e.AddRule(Normalize::Bitwise::Get_Rotate_ModuloBitWidth_Rule());

    return e;
}

// =========================================================
// Simplify - Bitwise
// =========================================================
inline RuleEngine BuildSimplifyBitwise() {
    RuleEngine e;

    // NOT
    e.AddRule(Simplify::Bitwise::Get_Not_Pushdown_Rule());
    e.AddRule(Simplify::Bitwise::Get_Not_Rule());
    e.AddRule(Simplify::Bitwise::Get_Not_Xor_Rule());

    // Cancel
    e.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());
    e.AddRule(Simplify::Bitwise::Get_And_Cancel_Rule());
    e.AddRule(Simplify::Bitwise::Get_Or_Cancel_Rule());

    // Reduction
    e.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());
    e.AddRule(Simplify::Bitwise::Get_Xor_Not_Reduction_Rule());
    e.AddRule(Simplify::Bitwise::Get_Xor_And_Reduction_Rule());

    // Fold
    e.AddRule(Simplify::Bitwise::Get_Xor_Fold_Rule());
    e.AddRule(Simplify::Bitwise::Get_And_Fold_Rule());
    e.AddRule(Simplify::Bitwise::Get_Or_Fold_Rule());

    // Neutral
    e.AddRule(Simplify::Bitwise::Get_Xor_Zero_Rule());

    // Structural
    e.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());
    e.AddRule(Simplify::Bitwise::Get_And_Idempotent_Rule());

    // Logical
    e.AddRule(Simplify::Bitwise::Get_Complement_Rule());

    // Dominance
    e.AddRule(Simplify::Bitwise::Get_And_Zero_Dominance_Rule());
    e.AddRule(Simplify::Bitwise::Get_And_One_Identity_Rule());
    e.AddRule(Simplify::Bitwise::Get_Or_One_Dominance_Rule());
    e.AddRule(Simplify::Bitwise::Get_Or_Zero_Identity_Rule());

    return e;
}

// =========================================================
// Simplify - Arithmetic
// =========================================================
inline RuleEngine BuildSimplifyArithmetic() {
    RuleEngine e;

    e.AddRule(Simplify::Arithmetic::Get_Add_Zero_Rule());
    e.AddRule(Simplify::Arithmetic::Get_Sub_Zero_Rule());
    e.AddRule(Simplify::Arithmetic::Get_Mul_One_Rule());
    e.AddRule(Simplify::Arithmetic::Get_Mul_Zero_Rule());
    e.AddRule(Simplify::Arithmetic::Get_Div_One_Rule());
    e.AddRule(Simplify::Arithmetic::Get_Mod_Zero_Guard_Rule());
    e.AddRule(Simplify::Arithmetic::Get_Shift_Zero_Rule());
    e.AddRule(Simplify::Arithmetic::Get_Rotate_Zero_Rule());
    e.AddRule(Simplify::Arithmetic::Get_Neg_Neg_Rule());
    e.AddRule(Simplify::Arithmetic::Get_Add_Fold_Rule());
    e.AddRule(Simplify::Arithmetic::Get_Const_Combine_Rule());

    return e;
}

// =========================================================
// Factorize - Bitwise
// =========================================================
inline RuleEngine BuildFactorizeBitwise() {
    RuleEngine e;

    e.AddRule(Factorize::Bitwise::Get_Xor_And_Rule());
    e.AddRule(Factorize::Bitwise::Get_Xor_Pair_Cancel_Rule());
    e.AddRule(Factorize::Bitwise::Get_And_Absorb_Rule());
    e.AddRule(Factorize::Bitwise::Get_Or_Absorb_Rule());
    e.AddRule(Factorize::Bitwise::Get_Distribute_Rule());

    return e;
}

// =========================================================
// Factorize - Arithmetic
// =========================================================
inline RuleEngine BuildFactorizeArithmetic() {
    RuleEngine e;

    e.AddRule(Factorize::Arithmetic::Get_Add_Linear_Multiplicity_Rule());
    e.AddRule(Factorize::Arithmetic::Get_Add_CommonFactor_Rule());
    e.AddRule(Factorize::Arithmetic::Get_Mul_CombineConstants_Rule());

    return e;
}

// =========================================================
// Compose helpers
// =========================================================
inline RuleEngine BuildSimplifyFull() {
    RuleEngine e;

    e.Merge(BuildNormalize());
    e.Merge(BuildSimplifyBitwise());
    e.Merge(BuildSimplifyArithmetic());

    return e;
}

inline RuleEngine BuildFactorizeFull() {
    RuleEngine e;

    e.Merge(BuildNormalize());
    e.Merge(BuildFactorizeBitwise());
    e.Merge(BuildFactorizeArithmetic());

    return e;
}

inline RuleEngine BuildExplore() {
    RuleEngine e;

    e.Merge(BuildNormalize());
    e.Merge(BuildSimplifyBitwise());
    e.Merge(BuildSimplifyArithmetic());
    e.Merge(BuildFactorizeBitwise());
    e.Merge(BuildFactorizeArithmetic());

    return e;
}

} // namespace BitFlow::Core::Rules