#pragma once

#include <cstdint>
#include <vector>

namespace BitFlow::Core::AST {
struct Expr;
}

namespace BitFlow::Core::Rules {

enum RuleFlags : uint32_t {
    None = 0,
    Expanding = 1u << 0,
    Factorizing = 1u << 1,
    Arithmetic = 1u << 2,
    UnsafeForModulo = 1u << 3
};

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
    Simplify_XorNotReduction,

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
    Factorize_MulCombineConstants,
    Factorize_AndAbsorb,
    Factorize_OrAbsorb,
    Factorize_Distribute
};

struct Rule {
    RuleId id;
    bool (*match)(const AST::Expr&);
    AST::Expr* (*rewrite)(AST::Expr&);
    int stage;

    uint32_t Id{0};
    const char* Name{nullptr};
    std::vector<uint32_t> Dependencies{};
    uint32_t Flags{RuleFlags::None};

    Rule(RuleId ruleId, bool (*ruleMatch)(const AST::Expr&), AST::Expr* (*ruleRewrite)(AST::Expr&), int ruleStage,
         std::vector<RuleId> deps = {}, uint32_t ruleFlags = RuleFlags::None, const char* ruleName = nullptr)
        : id(ruleId), match(ruleMatch), rewrite(ruleRewrite), stage(ruleStage), Id(static_cast<uint32_t>(ruleId)),
          Name(ruleName), Flags(ruleFlags) {
        Dependencies.reserve(deps.size());
        for (RuleId dep : deps) {
            Dependencies.push_back(static_cast<uint32_t>(dep));
        }
    }
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
Rule Get_Xor_Not_Reduction_Rule();
Rule Get_And_ZeroDominance_Rule();
Rule Get_And_OneIdentity_Rule();
Rule Get_Or_OneDominance_Rule();
Rule Get_Or_ZeroIdentity_Rule();
} // namespace Simplify::Bitwise

namespace Factorize::Arithmetic {
Rule Get_Add_CommonFactor_Rule();
Rule Get_Mul_CombineConstants_Rule();
} // namespace Factorize::Arithmetic

namespace Factorize::Bitwise {
Rule Get_Xor_And_Rule();
Rule Get_Xor_Pair_Cancel_Rule();
Rule Get_And_Absorb_Rule();
Rule Get_Or_Absorb_Rule();
Rule Get_Distribute_Rule();
} // namespace Factorize::Bitwise

} // namespace BitFlow::Core::Rules
