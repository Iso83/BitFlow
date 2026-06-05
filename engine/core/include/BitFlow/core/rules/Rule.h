#pragma once

#include <BitFlow/core/ids/ExprId.h>
#include <BitFlow/core/rules/RuleKey.h>
#include <vector>

namespace BitFlow::Core::Expression {
struct ExprStore;
} // namespace BitFlow::Core::Expression

namespace BitFlow::Core::Rules {
class RewriteContext;

struct Rule {
    RuleKey key;

    bool (*match)(const Expression::ExprStore*, const Expression::ExprNameMap*, Ids::ExprId);
    Ids::ExprId (*rewrite)(RewriteContext&, const Expression::ExprNameMap*, Ids::ExprId);

    std::vector<RuleKey> deps{};

    Rule(RuleKey k, bool (*m)(const Expression::ExprStore*, const Expression::ExprNameMap*, Ids::ExprId),
         Ids::ExprId (*r)(RewriteContext&, const Expression::ExprNameMap*, Ids::ExprId), std::vector<RuleKey> d = {})
        : key(std::move(k)), match(m), rewrite(r), deps(std::move(d)) {}
};

namespace Normalize {
inline constexpr RuleKey Flatten{"CORE.NORMALIZE.FLATTEN"};
Rule Get_Flatten_Rule();

inline constexpr RuleKey Order{"CORE.NORMALIZE.ORDER"};
Rule Get_Order_Rule();
} // namespace Normalize

namespace Normalize::Arithmetic {
inline constexpr RuleKey AddNegToSub{"CORE.NORMALIZE.ARITHMETIC.ADD_NEG_TO_SUB"};
Rule Get_AddNegToSub_Rule();
inline constexpr RuleKey SubToNeg{"CORE.NORMALIZE.ARITHMETIC.SUB_TO_NEG"};
Rule Get_SubToNeg_Rule();
} // namespace Normalize::Arithmetic

namespace Normalize::Bitwise {
inline constexpr RuleKey RotateModulo{"CORE.NORMALIZE.BITWISE.ROTATE_MODULO"};
Rule Get_RotateModulo_Rule();
} // namespace Normalize::Bitwise

namespace Simplify::Arithmetic {
inline constexpr RuleKey AddZero{"CORE.SIMPLIFY.ARITHMETIC.ADD_ZERO"};
Rule Get_AddZero_Rule();

inline constexpr RuleKey MulOne{"CORE.SIMPLIFY.ARITHMETIC.MUL_ONE"};
Rule Get_MulOne_Rule();

inline constexpr RuleKey PowOne{"CORE.SIMPLIFY.ARITHMETIC.POW_ONE"};
Rule Get_PowOne_Rule();

inline constexpr RuleKey MulZero{"CORE.SIMPLIFY.ARITHMETIC.MUL_ZERO"};
Rule Get_MulZero_Rule();

inline constexpr RuleKey PowZero{"CORE.SIMPLIFY.ARITHMETIC.POW_ZERO"};
Rule Get_PowZero_Rule();

inline constexpr RuleKey SubZero{"CORE.SIMPLIFY.ARITHMETIC.SUB_ZERO"};
Rule Get_SubZero_Rule();

inline constexpr RuleKey SubSelf{"CORE.SIMPLIFY.ARITHMETIC.SUB_SELF"};
Rule Get_SubSelf_Rule();

inline constexpr RuleKey DivOne{"CORE.SIMPLIFY.ARITHMETIC.DIV_ONE"};
Rule Get_DivOne_Rule();

inline constexpr RuleKey DivSelf{"CORE.SIMPLIFY.ARITHMETIC.DIV_SELF"};
Rule Get_DivSelf_Rule();

inline constexpr RuleKey ModOne{"CORE.SIMPLIFY.ARITHMETIC.MOD_ONE"};
Rule Get_ModOne_Rule();

inline constexpr RuleKey ModSelf{"CORE.SIMPLIFY.ARITHMETIC.MOD_SELF"};
Rule Get_ModSelf_Rule();

inline constexpr RuleKey ShiftZero{"CORE.SIMPLIFY.ARITHMETIC.SHIFT_ZERO"};
Rule Get_ShiftZero_Rule();

inline constexpr RuleKey RotateZero{"CORE.SIMPLIFY.ARITHMETIC.ROTATE_ZERO"};
Rule Get_RotateZero_Rule();

inline constexpr RuleKey ShiftRotateConstantFold{"CORE.SIMPLIFY.ARITHMETIC.SHIFT_ROTATE_CONSTANT_FOLD"};
Rule Get_ShiftRotateConstantFold_Rule();

inline constexpr RuleKey NegNeg{"CORE.SIMPLIFY.ARITHMETIC.NEG_NEG"};
Rule Get_NegNeg_Rule();

inline constexpr RuleKey NegPowEven{"CORE.SIMPLIFY.ARITHMETIC.NEG_POW_EVEN"};
Rule Get_NegPowEven_Rule();

inline constexpr RuleKey SubNeg{"CORE.SIMPLIFY.ARITHMETIC.SUB_NEG"};
Rule Get_SubNeg_Rule();

inline constexpr RuleKey AddFold{"CORE.SIMPLIFY.ARITHMETIC.ADD_FOLD"};
Rule Get_AddFold_Rule();

inline constexpr RuleKey SubConstFold{"CORE.SIMPLIFY.ARITHMETIC.SUB_CONSTANT_FOLD"};
Rule Get_SubConstFold_Rule();

inline constexpr RuleKey SubAddSelfCancel{"CORE.SIMPLIFY.ARITHMETIC.SUB_ADD_SELF_CANCEL"};
Rule Get_SubAddSelfCancel_Rule();

inline constexpr RuleKey SubMulLinearCancel{"CORE.SIMPLIFY.ARITHMETIC.SUB_MUL_LINEAR_CANCEL"};
Rule Get_SubMulLinearCancel_Rule();

inline constexpr RuleKey MulDivConstantReduction{"CORE.SIMPLIFY.ARITHMETIC.MUL_DIV_CONSTANT_REDUCTION"};
Rule Get_MulDivConstantReduction_Rule();

inline constexpr RuleKey MulToPow{"CORE.SIMPLIFY.ARITHMETIC.MUL_TO_POW"};
Rule Get_MulToPow_Rule();

inline constexpr RuleKey CombineMulPow{"CORE.SIMPLIFY.ARITHMETIC.COMBINE_MUL_POW"};
Rule Get_CombineMulPow_Rule();

inline constexpr RuleKey CombineConstants{"CORE.SIMPLIFY.ARITHMETIC.COMBINE_CONSTANTS"};
Rule Get_CombineConstants_Rule();
} // namespace Simplify::Arithmetic

namespace Simplify::Bitwise {
inline constexpr RuleKey XorZero{"CORE.SIMPLIFY.BITWISE.XOR_ZERO"};
Rule Get_XorZero_Rule();

inline constexpr RuleKey AndFold{"CORE.SIMPLIFY.BITWISE.AND_FOLD"};
Rule Get_AndFold_Rule();

inline constexpr RuleKey OrFold{"CORE.SIMPLIFY.BITWISE.OR_FOLD"};
Rule Get_OrFold_Rule();

inline constexpr RuleKey XorFold{"CORE.SIMPLIFY.BITWISE.XOR_FOLD"};
Rule Get_XorFold_Rule();

inline constexpr RuleKey XorCancel{"CORE.SIMPLIFY.BITWISE.XOR_CANCEL"};
Rule Get_XorCancel_Rule();

inline constexpr RuleKey Not{"CORE.SIMPLIFY.BITWISE.NOT"};
Rule Get_Not_Rule();

inline constexpr RuleKey NotPushdown{"CORE.SIMPLIFY.BITWISE.NOT_PUSHDOWN"};
Rule Get_NotPushdown_Rule();

inline constexpr RuleKey NotXor{"CORE.SIMPLIFY.BITWISE.NOT_XOR"};
Rule Get_NotXor_Rule();

inline constexpr RuleKey Idempotent{"CORE.SIMPLIFY.BITWISE.IDEMPOTENT"};
Rule Get_Idempotent_Rule();

inline constexpr RuleKey Complement{"CORE.SIMPLIFY.BITWISE.COMPLEMENT"};
Rule Get_Complement_Rule();

inline constexpr RuleKey AndXorReduction{"CORE.SIMPLIFY.BITWISE.AND_XOR_REDUCTION"};
Rule Get_AndXorReduction_Rule();

inline constexpr RuleKey XorAndReduction{"CORE.SIMPLIFY.BITWISE.XOR_AND_REDUCTION"};
Rule Get_XorAndReduction_Rule();

inline constexpr RuleKey XorAndNotReduction{"CORE.SIMPLIFY.BITWISE.XOR_AND_NOT_REDUCTION"};
Rule Get_XorAndNotReduction_Rule();
} // namespace Simplify::Bitwise

namespace Factorize::Arithmetic {
inline constexpr RuleKey AddLinearMultiplicity{"CORE.FACTORIZE.ARITHMETIC.ADD_LINEAR_MULTIPLICITY"};
Rule Get_AddLinearMultiplicity_Rule();

inline constexpr RuleKey AddCommonFactor{"CORE.FACTORIZE.ARITHMETIC.ADD_COMMON_FACTOR"};
Rule Get_AddCommonFactor_Rule();

inline constexpr RuleKey PerfectSquare{"CORE.FACTORIZE.ARITHMETIC.PERFECT_SQUARE"};
Rule Get_PerfectSquare_Rule();

inline constexpr RuleKey DifferenceOfSquares{"CORE.FACTORIZE.ARITHMETIC.DIFFERENCE_OF_SQUARES"};
Rule Get_DifferenceOfSquares_Rule();

inline constexpr RuleKey PromoteFactorsToPower{"CORE.FACTORIZE.ARITHMETIC.PROMOTE_FACTORS_TO_POWER"};
Rule Get_PromoteFactorsToPower_Rule();

inline constexpr RuleKey CommonFactorCancel_PowTerms{"CORE.FACTORIZE.ARITHMETIC.COMMON_FACTOR_CANCEL_POW_TERMS"};
Rule Get_CommonFactorCancel_PowTerms_Rule();

inline constexpr RuleKey CommonFactorCancel{"CORE.FACTORIZE.ARITHMETIC.COMMON_FACTOR_CANCEL"};
Rule Get_CommonFactorCancel_Rule();

inline constexpr RuleKey SubCommonDenominator{"CORE.FACTORIZE.ARITHMETIC.SUB_COMMON_DENOMINATOR"};
Rule Get_SubCommonDenominator_Rule();

inline constexpr RuleKey AddCommonDenominator{"CORE.FACTORIZE.ARITHMETIC.ADD_COMMON_DENOMINATOR"};
Rule Get_AddCommonDenominator_Rule();

inline constexpr RuleKey MulFractionNumerator{"CORE.FACTORIZE.ARITHMETIC.MUL_FRACTION_NUMERATOR"};
Rule Get_MulFractionNumerator_Rule();

inline constexpr RuleKey DivFractionDenominator{"CORE.FACTORIZE.ARITHMETIC.DIV_FRACTION_DENOMINATOR"};
Rule Get_DivFractionDenominator_Rule();
} // namespace Factorize::Arithmetic

namespace Factorize::Bitwise {
inline constexpr RuleKey XorAnd{"CORE.FACTORIZE.BITWISE.XOR_AND"};
Rule Get_XorAnd_Rule();

inline constexpr RuleKey AndAbsorb{"CORE.FACTORIZE.BITWISE.AND_ABSORB"};
Rule Get_AndAbsorb_Rule();

inline constexpr RuleKey OrAbsorb{"CORE.FACTORIZE.BITWISE.OR_ABSORB"};
Rule Get_OrAbsorb_Rule();

inline constexpr RuleKey Distribute{"CORE.FACTORIZE.BITWISE.DISTRIBUTE"};
Rule Get_Distribute_Rule();

inline constexpr RuleKey DistributeAndOverOr{"CORE.FACTORIZE.BITWISE.DISTRIBUTE_AND_OVER_OR"};
Rule Get_DistributeAndOverOr_Rule();
} // namespace Factorize::Bitwise

} // namespace BitFlow::Core::Rules
