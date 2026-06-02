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
    e.AddRule(Normalize::Arithmetic::Get_AddNegToSub_Rule());
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

    // Reduction
    e.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    e.AddRule(Simplify::Bitwise::Get_XorAndNotReduction_Rule());
    e.AddRule(Simplify::Bitwise::Get_XorAndReduction_Rule());

    // Structural
    e.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());

    // Fold
    e.AddRule(Simplify::Bitwise::Get_XorFold_Rule());
    e.AddRule(Simplify::Bitwise::Get_AndFold_Rule());
    e.AddRule(Simplify::Bitwise::Get_OrFold_Rule());

    // Logical
    e.AddRule(Simplify::Bitwise::Get_Complement_Rule());

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
    e.AddRule(Simplify::Arithmetic::Get_SubSelf_Rule());
    e.AddRule(Simplify::Arithmetic::Get_MulOne_Rule());
    e.AddRule(Simplify::Arithmetic::Get_PowOne_Rule());
    e.AddRule(Simplify::Arithmetic::Get_MulZero_Rule());
    e.AddRule(Simplify::Arithmetic::Get_PowZero_Rule());
    e.AddRule(Simplify::Arithmetic::Get_DivOne_Rule());
    e.AddRule(Simplify::Arithmetic::Get_DivSelf_Rule());
    e.AddRule(Simplify::Arithmetic::Get_ModOne_Rule());
    e.AddRule(Simplify::Arithmetic::Get_ModSelf_Rule());
    e.AddRule(Simplify::Arithmetic::Get_ShiftZero_Rule());
    e.AddRule(Simplify::Arithmetic::Get_RotateZero_Rule());
    e.AddRule(Simplify::Arithmetic::Get_NegNeg_Rule());
    e.AddRule(Simplify::Arithmetic::Get_NegPowEven_Rule());
    e.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    e.AddRule(Simplify::Arithmetic::Get_SubNeg_Rule());
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
    e.AddRule(Factorize::Bitwise::Get_AndAbsorb_Rule());
    e.AddRule(Factorize::Bitwise::Get_OrAbsorb_Rule());

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
    e.AddRule(Factorize::Arithmetic::Get_PerfectSquare_Rule());
    e.AddRule(Factorize::Arithmetic::Get_DifferenceOfSquares_Rule());
    e.AddRule(Factorize::Arithmetic::Get_PromoteFactorsToPower_Rule());
    e.AddRule(Factorize::Arithmetic::Get_CommonFactorCancel_PowTerms_Rule());
    e.AddRule(Factorize::Arithmetic::Get_CommonFactorCancel_Rule());
    e.AddRule(Factorize::Arithmetic::Get_SubCommonDenominator_Rule());
    e.AddRule(Factorize::Arithmetic::Get_AddCommonDenominator_Rule());
    e.AddRule(Factorize::Arithmetic::Get_MulFractionNumerator_Rule());
    e.AddRule(Factorize::Arithmetic::Get_DivFractionDenominator_Rule());

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

inline RuleEngine BuildExpand() {
    RuleEngine e;

    e.AddRule(Normalize::Get_Flatten_Rule());

    e.AddRule(Normalize::Arithmetic::Get_SubToNeg_Rule());

    e.AddRule(Factorize::Bitwise::Get_Distribute_Rule());

    return e;
}

} // namespace BitFlow::Core::Rules
