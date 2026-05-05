#pragma once

#include <BitFlow/core/ids/ExprId.h>
#include <BitFlow/core/rules/RuleKey.h>
#include <vector>

namespace BitFlow::Core::Expression {
struct ExprStore;
} // namespace BitFlow::Core::Expression

namespace BitFlow::Core::Rules {
struct Rule {
    RuleKey key;

    bool (*match)(const Expression::ExprStore*, Ids::ExprId);
    Ids::ExprId (*rewrite)(Expression::ExprStore*, Ids::ExprId);

    std::vector<RuleKey> deps{};

    Rule(RuleKey k, bool (*m)(const Expression::ExprStore*, Ids::ExprId),
         Ids::ExprId (*r)(Expression::ExprStore*, Ids::ExprId), std::vector<RuleKey> d = {})
        : key(std::move(k)), match(m), rewrite(r), deps(std::move(d)) {}
};

namespace Normalize {
inline constexpr RuleKey Flatten{"CORE.NORMALIZE.FLATTEN"};
Rule Get_Flatten_Rule();

inline constexpr RuleKey Order{"CORE.NORMALIZE.ORDER"};
Rule Get_Order_Rule();
} // namespace Normalize

namespace Simplify::Arithmetic {
inline constexpr RuleKey Add_Zero{"CORE.SIMPLIFY.ARITHMETIC.ADD_ZERO"};
Rule Get_Add_Zero_Rule();

inline constexpr RuleKey Mul_One{"CORE.SIMPLIFY.ARITHMETIC.MUL_ONE"};
Rule Get_Mul_One_Rule();

inline constexpr RuleKey Mul_Zero{"CORE.SIMPLIFY.ARITHMETIC.MUL_ZEO"};
Rule Get_Mul_Zero_Rule();

inline constexpr RuleKey Sub_Zero{"CORE.SIMPLIFY.ARITHMETIC.SUB_ZEO"};
Rule Get_Sub_Zero_Rule();

inline constexpr RuleKey Div_One{"CORE.SIMPLIFY.ARITHMETIC.DIV_ONE"};
Rule Get_Div_One_Rule();

inline constexpr RuleKey Mod_Zero_Guard{"CORE.SIMPLIFY.ARITHMETIC.MOD_ZERO_GUARD"};
Rule Get_Mod_Zero_Guard_Rule();

inline constexpr RuleKey Shift_Zero{"CORE.SIMPLIFY.ARITHMETIC.SHIFT_ZERO"};
Rule Get_Shift_Zero_Rule();

inline constexpr RuleKey Rotate_Zero{"CORE.SIMPLIFY.ARITHMETIC.ROTATE_ZERO"};
Rule Get_Rotate_Zero_Rule();

inline constexpr RuleKey Neg_Neg{"CORE.SIMPLIFY.ARITHMETIC.NEG_NEG"};
Rule Get_Neg_Neg_Rule();

inline constexpr RuleKey Add_Fold{"CORE.SIMPLIFY.ARITHMETIC.ADD_FOLD"};
Rule Get_Add_Fold_Rule();

inline constexpr RuleKey Const_Combine{"CORE.SIMPLIFY.ARITHMETIC.CONST_COMBINE"};
Rule Get_Const_Combine_Rule();
} // namespace Simplify::Arithmetic

namespace Simplify::Bitwise {
inline constexpr RuleKey Xor_Zero{"CORE.SIMPLIFY.BITWISE.XOR_ZERO"};
Rule Get_Xor_Zero_Rule();

inline constexpr RuleKey And_Fold{"CORE.SIMPLIFY.BITWISE.AND_FOLD"};
Rule Get_And_Fold_Rule();

inline constexpr RuleKey Or_Fold{"CORE.SIMPLIFY.BITWISE.OR_FOLD"};
Rule Get_Or_Fold_Rule();

inline constexpr RuleKey Xor_Fold{"CORE.SIMPLIFY.BITWISE.XOR_FOLD"};
Rule Get_Xor_Fold_Rule();

inline constexpr RuleKey And_Cancel{"CORE.SIMPLIFY.BITWISE.AND_CANCEL"};
Rule Get_And_Cancel_Rule();

inline constexpr RuleKey Or_Cancel{"CORE.SIMPLIFY.BITWISE.OR_CANCEL"};
Rule Get_Or_Cancel_Rule();

inline constexpr RuleKey Xor_Cancel{"CORE.SIMPLIFY.BITWISE.XOR_CANCEL"};
Rule Get_Xor_Cancel_Rule();

inline constexpr RuleKey Not{"CORE.SIMPLIFY.BITWISE.NOT"};
Rule Get_Not_Rule();

inline constexpr RuleKey Not_Pushdown{"CORE.SIMPLIFY.BITWISE.NOT_PUSHDOWN"};
Rule Get_Not_Pushdown_Rule();

inline constexpr RuleKey Not_Xor{"CORE.SIMPLIFY.BITWISE.NOT_XOR"};
Rule Get_Not_Xor_Rule();

inline constexpr RuleKey Idempotent{"CORE.SIMPLIFY.BITWISE.IDEMPOTENT"};
Rule Get_Idempotent_Rule();

inline constexpr RuleKey And_Idempotent{"CORE.SIMPLIFY.BITWISE.AND_IDEMPOTENT"};
Rule Get_And_Idempotent_Rule();

inline constexpr RuleKey Complement{"CORE.SIMPLIFY.BITWISE.COMPLEMENT"};
Rule Get_Complement_Rule();

inline constexpr RuleKey And_Xor_Reduction{"CORE.SIMPLIFY.BITWISE.AND_XOR_REDUCTION"};
Rule Get_And_Xor_Reduction_Rule();

inline constexpr RuleKey Xor_And_Reduction{"CORE.SIMPLIFY.BITWISE.XOR_AND_REDUCTION"};
Rule Get_Xor_And_Reduction_Rule();

inline constexpr RuleKey Xor_Not_Reduction{"CORE.SIMPLIFY.BITWISE.XOR_NOT_REDUCTION"};
Rule Get_Xor_Not_Reduction_Rule();

inline constexpr RuleKey And_Zero_Dominance{"CORE.SIMPLIFY.BITWISE.AND_ZERO_DOMINANCE"};
Rule Get_And_Zero_Dominance_Rule();

inline constexpr RuleKey And_One_Identity{"CORE.SIMPLIFY.BITWISE.AND_ONE_IDENTITY"};
Rule Get_And_One_Identity_Rule();

inline constexpr RuleKey Or_One_Dominance{"CORE.SIMPLIFY.BITWISE.OR_ONE_DOMINANCE"};
Rule Get_Or_One_Dominance_Rule();

inline constexpr RuleKey Or_Zero_Identity{"CORE.SIMPLIFY.BITWISE.OR_ZERO_IDENTITY"};
Rule Get_Or_Zero_Identity_Rule();
} // namespace Simplify::Bitwise

namespace Factorize::Arithmetic {
inline constexpr RuleKey Add_Linear_Multiplicity{"CORE.FACTORIZE.ARITHMETIC.ADD_LINEAR_MULTIPLICITY"};
Rule Get_Add_Linear_Multiplicity_Rule();

inline constexpr RuleKey Add_CommonFactor{"CORE.FACTORIZE.ARITHMETIC.ADD_COMMONFACTOR"};
Rule Get_Add_CommonFactor_Rule();

inline constexpr RuleKey Mul_CombineConstants{"CORE.FACTORIZE.ARITHMETIC.MUL_COMBINECONSTANTS"};
Rule Get_Mul_CombineConstants_Rule();
} // namespace Factorize::Arithmetic

namespace Factorize::Bitwise {
inline constexpr RuleKey Xor_And{"CORE.FACTORIZE.BITWISE.XOR_AND"};
Rule Get_Xor_And_Rule();

inline constexpr RuleKey Xor_Pair_Cancel{"CORE.FACTORIZE.BITWISE.XOR_PAIR_CANCEL"};
Rule Get_Xor_Pair_Cancel_Rule();

inline constexpr RuleKey And_Absorb{"CORE.FACTORIZE.BITWISE.AND_ABSORB"};
Rule Get_And_Absorb_Rule();

inline constexpr RuleKey Or_Absorb{"CORE.FACTORIZE.BITWISE.OR_ABSORB"};
Rule Get_Or_Absorb_Rule();

inline constexpr RuleKey Distribute{"CORE.FACTORIZE.BITWISE.DISTRIBUTE"};
Rule Get_Distribute_Rule();
} // namespace Factorize::Bitwise

} // namespace BitFlow::Core::Rules