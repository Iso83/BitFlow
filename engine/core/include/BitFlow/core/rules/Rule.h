#pragma once

#include <vector>

namespace BitFlow::Core::AST {
struct Expr;
}

namespace BitFlow::Core::Rules {

enum class RuleId {
    Normalize_Flatten,
    Normalize_Order,

    Simplify_AddZero,
    Simplify_SubZero,
    Simplify_MulZero,
    Simplify_ModZeroGuard,
    Simplify_ShiftZero,
    Simplify_RotateModuloBitwidth,

    Simplify_MulOne,
    Simplify_DivOne,
    Simplify_NegNeg,

    Simplify_XorZero,

    Simplify_AddFold,
    Simplify_ArithmeticConstCombine,
    Simplify_AndFold,
    Simplify_OrFold,
    Simplify_XorFold,

    Simplify_XorCancel,
    Simplify_AndCancel,
    Simplify_OrCancel,

    Simplify_Not,
    Simplify_NotPushdown,
    Simplify_NotXor,

    Simplify_Idempotent,
    Simplify_And_Idempotent,
    Simplify_Complement,
    Simplify_AndXorReduction,
    Simplify_XorAndReduction,

    // Simplify - Dominance / Identity
    Simplify_AndZeroDominance,
    Simplify_AndOneIdentity,
    Simplify_OrOneDominance,
    Simplify_OrZeroIdentity,

    Simplify_CH,

    Simplify_MAJ,

    Factorize_XorAnd,
    Factorize_XorPairCancel,
    Factorize_AddCommonFactor,
    Factorize_AndAbsorb,
    Factorize_OrAbsorb,
    Factorize_Distribute
};

struct Rule {
    RuleId id;
    bool (*match)(const AST::Expr&);
    AST::Expr* (*rewrite)(AST::Expr&);
    int stage;

    std::vector<RuleId> deps{};
};

namespace Normalize {
Rule Get_Flatten_Rule();
Rule Get_Order_Rule();
} // namespace Normalize

namespace Simplify::Arithmetic {
Rule Get_Add_Zero_Rule();
Rule Get_Mul_One_Rule();
Rule Get_Mul_Zero_Rule();
Rule Get_Sub_Zero_Rule();
Rule Get_Div_One_Rule();
Rule Get_Mod_Zero_Guard_Rule();
Rule Get_Shift_Zero_Rule();
Rule Get_Rotate_Modulo_Bitwidth_Rule();
Rule Get_Neg_Neg_Rule();

Rule Get_Add_Fold_Rule();
Rule Get_Const_Combine_Rule();
} // namespace Simplify::Arithmetic

namespace Simplify {
Rule Get_CH_Simplify_Rule();
Rule Get_MAJ_Simplify_Rule();
} // namespace Simplify

namespace Simplify::Bitwise {
Rule Get_Xor_Zero_Rule();
Rule Get_And_Fold_Rule();
Rule Get_Or_Fold_Rule();
Rule Get_Xor_Fold_Rule();
Rule Get_And_Cancel_Rule();
Rule Get_Or_Cancel_Rule();
Rule Get_Xor_Cancel_Rule();
Rule Get_Not_Rule();
Rule Get_NotPushdown_Rule();
Rule Get_Not_Xor_Rule();
Rule Get_Idempotent_Rule();
Rule Get_And_Idempotent_Rule();
Rule Get_Complement_Rule();
Rule Get_And_Xor_Reduction_Rule();
Rule Get_Xor_And_Reduction_Rule();
Rule Get_And_ZeroDominance_Rule();
Rule Get_And_OneIdentity_Rule();
Rule Get_Or_OneDominance_Rule();
Rule Get_Or_ZeroIdentity_Rule();
} // namespace Simplify::Bitwise

namespace Factorize::Arithmetic {
Rule Get_Add_CommonFactor_Rule();
}

namespace Factorize::Bitwise {
Rule Get_Xor_And_Rule();
Rule Get_Xor_Pair_Cancel_Rule();
Rule Get_And_Absorb_Rule();
Rule Get_Or_Absorb_Rule();
Rule Get_Distribute_Rule();
} // namespace Factorize::Bitwise

} // namespace BitFlow::Core::Rules
