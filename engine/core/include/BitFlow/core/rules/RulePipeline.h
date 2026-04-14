#pragma once

#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>

namespace BitFlow::Core::Rules {

inline void Add_Bitwise_Simplify_Pipeline(RuleEngine& engine) {

    // =========================================================
    // 1. Normalize (STRUCTURE CANONICALIZATION)
    //
    // Ensures a canonical structure for all expressions.
    // - Flatten is required for multi-input rules (e.g. XOR cancel)
    // - Order ensures deterministic comparisons (by id)
    //
    // Without this phase, later rules may behave incorrectly.
    // =========================================================
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());

    // =========================================================
    // 2. Simplify (LOCAL ALGEBRA REDUCTION)
    //
    // Performs local reductions on normalized expressions.
    // Assumes inputs are flattened and ordered.
    //
    // Dependencies:
    // - XOR cancel requires flatten
    // - Idempotent requires flatten
    // - Complement works best after cancel/idempotent
    // =========================================================

    // --- NOT Transformations ---
    engine.AddRule(Simplify::Bitwise::Get_NotPushdown_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Not_Rule());
    engine.AddRule(Simplify::Get_Not_Xor_Rule());

    // --- Cancellation / Deduplication ---
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Cancel_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Or_Cancel_Rule());

    // --- Constant Folding ---
    engine.AddRule(Simplify::Get_Xor_Fold_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Fold_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Or_Fold_Rule());

    // --- Neutral / Zero Rules ---
    engine.AddRule(Simplify::Bitwise::Get_Xor_Zero_Rule());

    // --- Structural Simplifications ---
    engine.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Idempotent_Rule());

    // --- Logical Completion ---
    engine.AddRule(Simplify::Bitwise::Get_Complement_Rule());

    // --- Simplify - Dominance ---
    engine.AddRule(Simplify::Bitwise::Get_And_ZeroDominance_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_OneIdentity_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Or_OneDominance_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Or_ZeroIdentity_Rule());

    // --- CH / MAJ Pattern Simplification ---
    engine.AddRule(Simplify::Get_CH_Simplify_Rule());
    engine.AddRule(Simplify::Get_MAJ_Simplify_Rule());

    // =========================================================
    // 3. Factorize (STRUCTURE REWRITE)
    //
    // Rewrites expressions to expose new simplification opportunities.
    //
    // Notes:
    // - Should not cause expression explosion
    // - Works best after simplification (less noise)
    // - Distribute is intentionally excluded by default
    // =========================================================

    // --- Common Factor Extraction ---
    engine.AddRule(Factorize::Get_Xor_And_Rule());

    // --- Pattern-Based Cancellation ---
    engine.AddRule(Factorize::Get_Xor_Pair_Cancel_Rule());

    // --- Absorption Laws ---
    engine.AddRule(Factorize::Get_And_Absorb_Rule());
    engine.AddRule(Factorize::Get_Or_Absorb_Rule());

    // --- Optional (disabled by default due to growth) ---
    // engine.AddRule(Factorize::Get_Distribute_Rule());
}

} // namespace BitFlow::Core::Rules
