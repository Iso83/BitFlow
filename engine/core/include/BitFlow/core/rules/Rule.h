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
    Simplify_XorZero,

    Simplify_AddFold,
    Simplify_AndFold,
    Simplify_OrFold,
    Simplify_XorFold,

    Simplify_XorCancel,
    Simplify_XorDuplicateCancel,
    Simplify_AndCancel,
    Simplify_OrCancel,

    Simplify_Not,
    Simplify_NotPushdown,
    Simplify_NotXor,

    Simplify_Idempotent,
    Simplify_Complement,

    Factorize_XorAnd,
    Factorize_XorPairCancel,
    Factorize_AndAbsorb,
    Factorize_OrAbsorb,
    Factorize_AndDistribute
};

struct Rule {
    RuleId id;
    bool (*match)(const AST::Expr&);
    AST::Expr* (*rewrite)(AST::Expr&);
    int stage;

    std::vector<RuleId> deps{};
};

namespace Normalize {

#pragma region Flattening
/// Flattening (Associativity)
/// Removes nested same-operator nodes.
/// (a ^ (b ^ c)) → (a ^ b ^ c)
Rule Get_Flatten_Rule();
#pragma endregion

#pragma region Ordering
/// Ordering (Canonical Form)
/// Ensures deterministic ordering of commutative inputs.
/// a ^ b == b ^ a → sorted form
Rule Get_Order_Rule();
#pragma endregion

} // namespace Normalize

namespace Simplify {
#pragma region Identity & Neutral Elements
/// Identity / Neutral Elements
/// Removes neutral operands.
/// a + 0 = a
/// a ^ 0 = a
Rule Get_Add_Zero_Rule(); // deps: Normalize_Flatten
Rule Get_Xor_Zero_Rule(); // deps: Normalize_Flatten
#pragma endregion

#pragma region Constant Folding
/// Constant Folding
/// Evaluates expressions with constant inputs.
Rule Get_Add_Fold_Rule(); // deps: Normalize_Flatten
Rule Get_And_Fold_Rule(); // deps: Normalize_Flatten
Rule Get_Or_Fold_Rule();  // deps: Normalize_Flatten
Rule Get_Xor_Fold_Rule(); // deps: Normalize_Flatten
#pragma endregion

#pragma region Cancellation
/// Cancellation
/// Eliminates duplicate operands.
/// a ^ a = 0
/// a & a = a
/// a | a = a
Rule Get_Xor_Cancel_Rule();          // deps: Normalize_Flatten
Rule Get_Xor_DuplicateCancel_Rule(); // deps: Normalize_Flatten
Rule Get_And_Cancel_Rule();          // deps: Normalize_Flatten
Rule Get_Or_Cancel_Rule();           // deps: Normalize_Flatten
#pragma endregion

#pragma region NOT Transformations
/// NOT Transformations
/// Pushdown and normalization of negations.
Rule Get_Not_Rule();
Rule Get_NotPushdown_Rule(); // deps: Simplify_Not
Rule Get_Not_Xor_Rule();     // deps: Normalize_Flatten
#pragma endregion

#pragma region Idempotent Laws
/// Idempotent Laws
/// Duplicate inputs collapse.
/// a & a = a
/// a | a = a
Rule Get_Idempotent_Rule(); // deps: Normalize_Flatten
#pragma endregion

#pragma region Complement Laws
/// Complement Laws
/// Opposites eliminate or saturate.
/// a & ~a = 0
/// a | ~a = 1
Rule Get_Complement_Rule(); // deps: Normalize_Flatten, Simplify_Idempotent
#pragma endregion
} // namespace Simplify

namespace Factorize {

#pragma region Common Factor Extraction
/// Common Factor Extraction
/// Pulls a shared operand out of multiple terms.
/// (a & b) ^ (a & c) → a & (b ^ c)
Rule Get_Xor_And_Rule(); // deps: Normalize_Flatten
#pragma endregion

#pragma region Cancellation (Factorized Forms)
/// Cancellation (Factorized XOR patterns)
/// Eliminates shared terms across XOR branches.
/// (a ^ b) ^ (a ^ c) → b ^ c
Rule Get_Xor_Pair_Cancel_Rule(); // deps: Normalize_Flatten, Simplify_XorCancel
#pragma endregion

#pragma region Absorption Laws
/// Absorption Laws
/// Removes redundant terms using dominance relations.
/// a & (a | b) → a
/// a | (a & b) → a
Rule Get_And_Absorb_Rule(); // deps: Normalize_Flatten
Rule Get_Or_Absorb_Rule();  // deps: Normalize_Flatten
#pragma endregion

#pragma region Distributive Laws
/// Distributive Laws
/// Expands expressions by distributing operators.
/// a & (b | c) → (a & b) | (a & c)
/// NOTE: increases expression size → use with care
Rule Get_And_Distribute_Rule(); // deps: Normalize_Flatten
#pragma endregion

} // namespace Factorize

} // namespace BitFlow::Core::Rules