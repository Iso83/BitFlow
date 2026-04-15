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
#pragma endregion

#pragma region Constant Folding
/// Constant Folding
/// Evaluates expressions with constant inputs.
Rule Get_Add_Fold_Rule(); // deps: Normalize_Flatten
#pragma endregion

#pragma region Cancellation
/// Cancellation
/// Eliminates duplicate operands.
/// a ^ a = 0
/// a & a = a
/// a | a = a
#pragma endregion

#pragma region NOT Transformations
/// NOT Transformations
/// Pushdown and normalization of negations.
#pragma endregion

#pragma region Idempotent Laws
/// Idempotent Laws
/// Duplicate inputs collapse.
/// a & a = a
/// a | a = a
#pragma endregion

#pragma region Complement Laws
/// Complement Laws
/// Opposites eliminate or saturate.
/// a & ~a = 0
/// a | ~a = 1
#pragma endregion

#pragma region Dominance & Identity
/// Dominance & Identity
/// Eliminates expressions using constant dominance laws.
/// a & 0 = 0
/// a & 1 = a
/// a | 1 = 1
/// a | 0 = a
#pragma endregion

#pragma region CH Pattern Simplification
/// CH Pattern Simplification
/// Recognizes CH-like bitselect patterns and applies direct reductions.
///
/// Canonical CH form:
/// (x & y) ^ (~x & z)
///
/// Simplifications:
/// - CH(x, y, y) = y
/// - CH(x, x, z) = x | z
/// - CH(x, y, x) = x & y
/// - CH(x, y, ~y) = ~(x ^ y)
///
/// Notes:
/// - Requires flattened and ordered input for stable matching
/// - Matches XOR of two AND branches
/// - One branch must contain x, the other must contain ~x
Rule Get_CH_Simplify_Rule(); // deps: Normalize_Flatten, Normalize_Order
#pragma endregion

#pragma region MAJ Pattern Simplification
/// MAJ Pattern Simplification
/// Recognizes majority patterns:
/// (x & y) ^ (x & z) ^ (y & z)
///
/// Simplifications:
/// - MAJ(x, x, y) = x
/// - MAJ(x, y, y) = y
///
/// Notes:
/// - Requires flattened and ordered XOR input
/// - Accepts collapsed forms such as:
///   x ^ (x & y) ^ (x & y)
Rule Get_MAJ_Simplify_Rule(); // deps: Normalize_Flatten, Normalize_Order
#pragma endregion
} // namespace Simplify

namespace Simplify::Bitwise {
#pragma region Identity & Neutral Elements
Rule Get_Xor_Zero_Rule(); // deps: Normalize_Flatten
#pragma endregion

#pragma region Constant Folding
Rule Get_And_Fold_Rule(); // deps: Normalize_Flatten
Rule Get_Or_Fold_Rule();  // deps: Normalize_Flatten
Rule Get_Xor_Fold_Rule(); // deps: Normalize_Flatten
#pragma endregion

#pragma region Cancellation
Rule Get_And_Cancel_Rule(); // deps: Normalize_Flatten
Rule Get_Or_Cancel_Rule();  // deps: Normalize_Flatten
Rule Get_Xor_Cancel_Rule(); // deps: Normalize_Flatten, Normalize_Order
#pragma endregion

#pragma region NOT Transformations
Rule Get_Not_Rule();
Rule Get_NotPushdown_Rule();
Rule Get_Not_Xor_Rule(); // deps: Normalize_Flatten
#pragma endregion

#pragma region Idempotent Laws
Rule Get_Idempotent_Rule();     // deps: Normalize_Flatten
Rule Get_And_Idempotent_Rule(); // deps: Normalize_Flatten
#pragma endregion

#pragma region Complement Laws
Rule Get_Complement_Rule(); // deps: Normalize_Flatten, Simplify_Idempotent
#pragma endregion

#pragma region Dominance & Identity
Rule Get_And_ZeroDominance_Rule(); // deps: Normalize_Flatten
Rule Get_And_OneIdentity_Rule();   // deps: Normalize_Flatten
Rule Get_Or_OneDominance_Rule();   // deps: Normalize_Flatten
Rule Get_Or_ZeroIdentity_Rule();   // deps: Normalize_Flatten
#pragma endregion
} // namespace Simplify::Bitwise

namespace Factorize::Bitwise {

#pragma region Common Factor Extraction
/// Common Factor Extraction
/// Pulls a shared operand out of XOR branches that are AND terms.
///
/// Example:
/// (a & b) ^ (a & c) → a & (b ^ c)
///
/// Also preserves untouched XOR terms:
/// (a & b) ^ (a & c) ^ d → (a & (b ^ c)) ^ d
///
/// Notes:
/// - Requires flattened and ordered input for stable matching
/// - Only factors operands that occur in at least 2 AND branches
/// - Does not yet handle cases like: a ^ (a & b)
Rule Get_Xor_And_Rule(); // deps: Normalize_Flatten, Normalize_Order
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
Rule Get_Distribute_Rule(); // deps: Normalize_Flatten
#pragma endregion

} // namespace Factorize::Bitwise

} // namespace BitFlow::Core::Rules
