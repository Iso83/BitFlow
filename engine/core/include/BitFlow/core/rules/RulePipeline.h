#pragma once

#include <BitFlow/core/rules/RuleEngine.h>

namespace BitFlow::Core::Rules {

// =========================================================
// Normalize
// =========================================================
inline RuleEngine BuildNormalize() {
    RuleEngine e;

    e.AddRule(Normalize::Get_Flatten_Rule());
    e.AddRule(Normalize::Get_Order_Rule());
    e.AddRule(Normalize::Bitwise::Get_RotateModulo_Rule());

    return e;
}

// =========================================================
// Simplify - Bitwise
// =========================================================
inline RuleEngine BuildSimplifyBitwise() {
    RuleEngine e;

    e.Merge(BuildNormalize());

    // NOT
    e.AddRule(Simplify::Bitwise::Get_NotPushdown_Rule());
    e.AddRule(Simplify::Bitwise::Get_Not_Rule());
    e.AddRule(Simplify::Bitwise::Get_NotXor_Rule());

    // Neutral
    e.AddRule(Simplify::Bitwise::Get_XorZero_Rule());

    // Cancel
    e.AddRule(Simplify::Bitwise::Get_XorCancel_Rule());
    e.AddRule(Simplify::Bitwise::Get_AndCancel_Rule());
    e.AddRule(Simplify::Bitwise::Get_OrCancel_Rule());

    // Reduction
    e.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    e.AddRule(Simplify::Bitwise::Get_XorNotReduction_Rule());
    e.AddRule(Simplify::Bitwise::Get_XorAndReduction_Rule());

    // Fold
    e.AddRule(Simplify::Bitwise::Get_XorFold_Rule());
    e.AddRule(Simplify::Bitwise::Get_AndFold_Rule());
    e.AddRule(Simplify::Bitwise::Get_OrFold_Rule());

    // Structural
    e.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());
    e.AddRule(Simplify::Bitwise::Get_AndIdempotent_Rule());

    // Logical
    e.AddRule(Simplify::Bitwise::Get_Complement_Rule());

    // Dominance
    e.AddRule(Simplify::Bitwise::Get_AndZero_Rule());
    e.AddRule(Simplify::Bitwise::Get_AndOneIdentity_Rule());
    e.AddRule(Simplify::Bitwise::Get_OrOneDominance_Rule());
    e.AddRule(Simplify::Bitwise::Get_OrZero_Rule());

    return e;
}

// =========================================================
// Simplify - Arithmetic
// =========================================================
inline RuleEngine BuildSimplifyArithmetic() {
    RuleEngine e;

    e.Merge(BuildNormalize());

    e.AddRule(Simplify::Arithmetic::Get_AddZero_Rule());
    e.AddRule(Simplify::Arithmetic::Get_SubZero_Rule());
    e.AddRule(Simplify::Arithmetic::Get_MulOne_Rule());
    e.AddRule(Simplify::Arithmetic::Get_MulZero_Rule());
    e.AddRule(Simplify::Arithmetic::Get_DivOne_Rule());
    e.AddRule(Simplify::Arithmetic::Get_ModZeroGuard_Rule());
    e.AddRule(Simplify::Arithmetic::Get_ShiftZero_Rule());
    e.AddRule(Simplify::Arithmetic::Get_RotateZero_Rule());
    e.AddRule(Simplify::Arithmetic::Get_NegNeg_Rule());
    e.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    e.AddRule(Simplify::Arithmetic::Get_SubConstFold_Rule());
    e.AddRule(Simplify::Arithmetic::Get_SubAddSelfCancel_Rule());
    e.AddRule(Simplify::Arithmetic::Get_SubMulLinearCancel_Rule());
    e.AddRule(Simplify::Arithmetic::Get_MulDivConstantReduction_Rule());
    e.AddRule(Simplify::Arithmetic::Get_CombineMulPow_Rule());
    e.AddRule(Simplify::Arithmetic::Get_MulToPow_Rule());
    e.AddRule(Simplify::Arithmetic::Get_CombineConstants_Rule());

    return e;
}

// =========================================================
// Factorize - Bitwise
// =========================================================
inline RuleEngine BuildFactorizeBitwise() {
    RuleEngine e;

    e.Merge(BuildNormalize());

    e.AddRule(Factorize::Bitwise::Get_XorAnd_Rule());
    e.AddRule(Factorize::Bitwise::Get_XorPairCancel_Rule());
    e.AddRule(Factorize::Bitwise::Get_AndAbsorb_Rule());
    e.AddRule(Factorize::Bitwise::Get_OrAbsorb_Rule());
    e.AddRule(Factorize::Bitwise::Get_Distribute_Rule());

    return e;
}

// =========================================================
// Factorize - Arithmetic
// =========================================================
inline RuleEngine BuildFactorizeArithmetic() {
    RuleEngine e;

    e.Merge(BuildNormalize());

    e.AddRule(Factorize::Arithmetic::Get_AddLinearMultiplicity_Rule());
    e.AddRule(Factorize::Arithmetic::Get_AddCommonFactor_Rule());
    e.AddRule(Factorize::Arithmetic::Get_CommonFactorCancel_PowTerms_Rule());
    e.AddRule(Factorize::Arithmetic::Get_MulCombineConstants_Rule());

    return e;
}

// =========================================================
// Compose helpers
// =========================================================
inline RuleEngine BuildSimplifyFull() {
    RuleEngine e;

    e.Merge(BuildSimplifyBitwise());
    e.Merge(BuildSimplifyArithmetic());

    return e;
}

inline RuleEngine BuildFactorizeFull() {
    RuleEngine e;

    e.Merge(BuildFactorizeBitwise());
    e.Merge(BuildFactorizeArithmetic());

    return e;
}

inline RuleEngine BuildExplore() {
    RuleEngine e;

    e.Merge(BuildSimplifyBitwise());
    e.Merge(BuildSimplifyArithmetic());
    e.Merge(BuildFactorizeBitwise());
    e.Merge(BuildFactorizeArithmetic());

    return e;
}

} // namespace BitFlow::Core::Rules
