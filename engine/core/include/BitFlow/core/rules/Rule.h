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

    Simplify_XorZero,

    Simplify_AddFold,
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

    // Simplify - Dominance / Identity
    Simplify_AndZeroDominance,
    Simplify_AndOneIdentity,
    Simplify_OrOneDominance,
    Simplify_OrZeroIdentity,

    Simplify_CH,

    Simplify_MAJ,

    Factorize_XorAnd,
    Factorize_XorPairCancel,
    Factorize_AndAbsorb,
    Factorize_OrAbsorb,
    Factorize_Distribute
};

struct Rule {
    RuleId id;
    bool (*match)(const AST::Expr&);
    // Rewrite contract:
    // - return nullptr when no rewrite is applied
    // - return a replacement Expr* when rewritten (preferred: fresh node)
    // - returning the same input pointer is treated as a no-op by the engine
    AST::Expr* (*rewrite)(AST::Expr&);
    int stage;

    std::vector<RuleId> deps{};
};

namespace Normalize {

// Flatten (associativity)
Rule Get_Flatten_Rule();

// Order (canonical commutative ordering)
Rule Get_Order_Rule();

} // namespace Normalize

namespace Simplify::Arithmetic {

// Identity / neutral (arithmetic)
Rule Get_Add_Zero_Rule(); // deps: Normalize_Flatten
Rule Get_Mul_One_Rule();  // deps: Normalize_Flatten
Rule Get_Mul_Zero_Rule(); // deps: Normalize_Flatten
Rule Get_Sub_Zero_Rule(); // deps: Normalize_Flatten
Rule Get_Div_One_Rule();  // deps: Normalize_Flatten
Rule Get_Mod_Zero_Guard_Rule(); // deps: Normalize_Flatten
Rule Get_Shift_Zero_Rule(); // deps: Normalize_Flatten
Rule Get_Rotate_Modulo_Bitwidth_Rule(); // deps: Normalize_Flatten

// Constant folding (arithmetic)
Rule Get_Add_Fold_Rule(); // deps: Normalize_Flatten

} // namespace Simplify::Arithmetic

namespace Simplify {

// SHA patterns
Rule Get_CH_Simplify_Rule();  // deps: Normalize_Flatten, Normalize_Order
Rule Get_MAJ_Simplify_Rule(); // deps: Normalize_Flatten, Normalize_Order

} // namespace Simplify

namespace Simplify::Bitwise {

// Identity / neutral
Rule Get_Xor_Zero_Rule(); // deps: Normalize_Flatten

// Constant folding
Rule Get_And_Fold_Rule(); // deps: Normalize_Flatten
Rule Get_Or_Fold_Rule();  // deps: Normalize_Flatten
Rule Get_Xor_Fold_Rule(); // deps: Normalize_Flatten

// Cancellation
Rule Get_And_Cancel_Rule(); // deps: Normalize_Flatten
Rule Get_Or_Cancel_Rule();  // deps: Normalize_Flatten
Rule Get_Xor_Cancel_Rule(); // deps: Normalize_Flatten, Normalize_Order

// NOT transforms
Rule Get_Not_Rule();
Rule Get_NotPushdown_Rule();
Rule Get_Not_Xor_Rule(); // deps: Normalize_Flatten

// Idempotent
Rule Get_Idempotent_Rule();     // deps: Normalize_Flatten
Rule Get_And_Idempotent_Rule(); // deps: Normalize_Flatten

// Complement
Rule Get_Complement_Rule(); // deps: Normalize_Flatten, Simplify_Idempotent

// Dominance / identity
Rule Get_And_ZeroDominance_Rule(); // deps: Normalize_Flatten
Rule Get_And_OneIdentity_Rule();   // deps: Normalize_Flatten
Rule Get_Or_OneDominance_Rule();   // deps: Normalize_Flatten
Rule Get_Or_ZeroIdentity_Rule();   // deps: Normalize_Flatten

} // namespace Simplify::Bitwise

namespace Factorize::Bitwise {

// Common factor
Rule Get_Xor_And_Rule(); // deps: Normalize_Flatten, Normalize_Order

// XOR pair cancel
Rule Get_Xor_Pair_Cancel_Rule(); // deps: Normalize_Flatten, Simplify_XorCancel

// Absorption
Rule Get_And_Absorb_Rule(); // deps: Normalize_Flatten
Rule Get_Or_Absorb_Rule();  // deps: Normalize_Flatten

// Distribute
Rule Get_Distribute_Rule(); // deps: Normalize_Flatten

} // namespace Factorize::Bitwise

} // namespace BitFlow::Core::Rules
