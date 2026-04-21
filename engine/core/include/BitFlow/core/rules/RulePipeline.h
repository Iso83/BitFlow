#pragma once

#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace BitFlow::Core::Rules {

enum class RuleProfile {
    normalize,
    simplify_bitwise,
    simplify_arithmetic,
    simplify_full_safe,
    factorize_bitwise_safe,
    factorize_arithmetic_safe,
    factorize_full_safe,
    expand_bitwise,
    explore
};

// =========================================================
// Normalize (global)
// =========================================================
inline void Add_Normalize_Rules(RuleEngine& engine) {
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
}

// =========================================================
// Simplify (Bitwise)
// =========================================================
inline void Add_Simplify_Bitwise_Rules(RuleEngine& engine) {

    // NOT
    engine.AddRule(Simplify::Bitwise::Get_NotPushdown_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Not_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Not_Xor_Rule());

    // Cancel
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Cancel_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Or_Cancel_Rule());

    // Reduction
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Not_Reduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_And_Reduction_Rule());

    // Fold
    engine.AddRule(Simplify::Bitwise::Get_Xor_Fold_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Fold_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Or_Fold_Rule());

    // Neutral
    engine.AddRule(Simplify::Bitwise::Get_Xor_Zero_Rule());

    // Structural
    engine.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Idempotent_Rule());

    // Logical
    engine.AddRule(Simplify::Bitwise::Get_Complement_Rule());

    // Dominance
    engine.AddRule(Simplify::Bitwise::Get_And_ZeroDominance_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_OneIdentity_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Or_OneDominance_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Or_ZeroIdentity_Rule());
}

// =========================================================
// Factorize (Bitwise)
// =========================================================
inline void Add_Factorize_Bitwise_Rules(RuleEngine& engine) {

    engine.AddRule(Factorize::Bitwise::Get_Xor_And_Rule());
    engine.AddRule(Factorize::Bitwise::Get_Xor_Pair_Cancel_Rule());

    engine.AddRule(Factorize::Bitwise::Get_And_Absorb_Rule());
    engine.AddRule(Factorize::Bitwise::Get_Or_Absorb_Rule());

    engine.AddRule(Factorize::Bitwise::Get_Distribute_Rule());
}

inline void Add_Factorize_Bitwise_Safe_Rules(RuleEngine& engine) {
    engine.AddRule(Factorize::Bitwise::Get_Xor_And_Rule());
    engine.AddRule(Factorize::Bitwise::Get_Xor_Pair_Cancel_Rule());
    engine.AddRule(Factorize::Bitwise::Get_And_Absorb_Rule());
    engine.AddRule(Factorize::Bitwise::Get_Or_Absorb_Rule());
}

inline void Add_Expand_Bitwise_Rules(RuleEngine& engine) {
    engine.AddRule(Factorize::Bitwise::Get_Distribute_Rule());
}

// =========================================================
// Simplify (Arithmetic)
// =========================================================
inline void Add_Simplify_Arithmetic_Rules(RuleEngine& engine) {
    engine.AddRule(Simplify::Arithmetic::Get_Add_Zero_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Sub_Zero_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Mul_One_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Mul_Zero_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Div_One_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Mod_Zero_Guard_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Shift_Zero_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Rotate_Modulo_Bitwidth_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Neg_Neg_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Add_Fold_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Const_Combine_Rule());
}

// =========================================================
// Factorize (Arithmetic)
// =========================================================
inline void Add_Factorize_Arithmetic_Safe_Rules(RuleEngine& engine) {
    engine.AddRule(Factorize::Arithmetic::Get_Add_LinearMultiplicity_Rule());
    engine.AddRule(Factorize::Arithmetic::Get_Add_CommonFactor_Rule());
    engine.AddRule(Factorize::Arithmetic::Get_Mul_CombineConstants_Rule());
}

inline void Add_Factorize_Arithmetic_Rules(RuleEngine& engine) {
    Add_Factorize_Arithmetic_Safe_Rules(engine);
}

// =========================================================
// Simplify (SHA - optional)
// =========================================================
inline void Add_Simplify_SHA_Rules(RuleEngine& engine) {
    engine.AddRule(Simplify::Get_CH_Simplify_Rule());
    engine.AddRule(Simplify::Get_MAJ_Simplify_Rule());
}

inline RuleEngine BuildProfile(RuleProfile profile) {
    RuleEngine engine;

    switch (profile) {
    case RuleProfile::normalize:
        Add_Normalize_Rules(engine);
        break;
    case RuleProfile::simplify_bitwise:
        Add_Normalize_Rules(engine);
        Add_Simplify_Bitwise_Rules(engine);
        break;
    case RuleProfile::simplify_arithmetic:
        Add_Normalize_Rules(engine);
        Add_Simplify_Arithmetic_Rules(engine);
        break;
    case RuleProfile::simplify_full_safe:
        Add_Normalize_Rules(engine);
        Add_Simplify_Bitwise_Rules(engine);
        Add_Simplify_Arithmetic_Rules(engine);
        Add_Simplify_SHA_Rules(engine);
        break;
    case RuleProfile::factorize_bitwise_safe:
        Add_Normalize_Rules(engine);
        engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());
        Add_Factorize_Bitwise_Safe_Rules(engine);
        break;
    case RuleProfile::factorize_arithmetic_safe:
        Add_Normalize_Rules(engine);
        Add_Factorize_Arithmetic_Safe_Rules(engine);
        break;
    case RuleProfile::factorize_full_safe:
        Add_Normalize_Rules(engine);
        engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());
        Add_Factorize_Bitwise_Safe_Rules(engine);
        Add_Factorize_Arithmetic_Safe_Rules(engine);
        break;
    case RuleProfile::expand_bitwise:
        Add_Normalize_Rules(engine);
        Add_Expand_Bitwise_Rules(engine);
        break;
    case RuleProfile::explore:
        Add_Normalize_Rules(engine);
        Add_Simplify_Bitwise_Rules(engine);
        Add_Simplify_Arithmetic_Rules(engine);
        Add_Simplify_SHA_Rules(engine);
        Add_Factorize_Bitwise_Safe_Rules(engine);
        Add_Factorize_Arithmetic_Safe_Rules(engine);
        break;
    }

    return engine;
}

inline RuleEngine BuildProfile(std::string_view profileName) {
    if (profileName == "normalize")
        return BuildProfile(RuleProfile::normalize);
    if (profileName == "simplify_bitwise")
        return BuildProfile(RuleProfile::simplify_bitwise);
    if (profileName == "simplify_arithmetic")
        return BuildProfile(RuleProfile::simplify_arithmetic);
    if (profileName == "simplify_full_safe")
        return BuildProfile(RuleProfile::simplify_full_safe);
    if (profileName == "factorize_bitwise_safe")
        return BuildProfile(RuleProfile::factorize_bitwise_safe);
    if (profileName == "factorize_arithmetic_safe")
        return BuildProfile(RuleProfile::factorize_arithmetic_safe);
    if (profileName == "factorize_full_safe")
        return BuildProfile(RuleProfile::factorize_full_safe);
    if (profileName == "expand_bitwise")
        return BuildProfile(RuleProfile::expand_bitwise);
    if (profileName == "explore")
        return BuildProfile(RuleProfile::explore);

    // Backward-compatible aliases.
    if (profileName == "sha_safe")
        return BuildProfile(RuleProfile::simplify_full_safe);
    if (profileName == "factorize")
        return BuildProfile(RuleProfile::factorize_full_safe);

    throw std::runtime_error("Unknown rule profile");
}

struct PipelineValidationResult {
    bool valid{true};
    std::vector<std::string> errors{};
};

inline PipelineValidationResult ValidateEngine(const RuleEngine& engine) {
    PipelineValidationResult result{};

    const auto& stages = engine.Stages();
    const auto& stageOrder = engine.StageOrder();

    if (stages.size() != stageOrder.size()) {
        result.valid = false;
        result.errors.emplace_back("Stage metadata mismatch: stages.size != stageOrder.size");
        return result;
    }

    std::unordered_set<uint32_t> seenIds;
    std::unordered_set<std::string_view> seenNames;

    int prevStage = std::numeric_limits<int>::min();
    for (size_t i = 0; i < stages.size(); ++i) {
        const int stageNumber = stageOrder[i];
        if (stageNumber < prevStage) {
            result.valid = false;
            result.errors.emplace_back("Stage regression detected");
        }
        prevStage = stageNumber;

        bool hasExpandOnly = false;
        bool hasFactorOnly = false;

        for (const auto& rule : stages[i].rules) {
            if (!seenIds.insert(rule.Id).second) {
                result.valid = false;
                result.errors.emplace_back("Duplicate rule id detected");
            }

            if (rule.Name == nullptr || rule.Name[0] == '\0') {
                result.valid = false;
                result.errors.emplace_back("Rule with empty name detected");
            } else if (!seenNames.insert(rule.Name).second) {
                result.valid = false;
                result.errors.emplace_back("Duplicate rule name detected");
            }

            const bool isExpanding = rule.IsExpanding();
            const bool isFactorizing = rule.IsFactorizing();
            if (isExpanding && !isFactorizing)
                hasExpandOnly = true;
            if (isFactorizing && !isExpanding)
                hasFactorOnly = true;
        }

        if (hasExpandOnly && hasFactorOnly) {
            result.valid = false;
            result.errors.emplace_back("Forbidden mix in stage: expanding + factorizing");
        }
    }

    std::unordered_set<uint32_t> allRuleIds;
    allRuleIds.reserve(seenIds.size());
    for (const auto& stage : stages) {
        for (const auto& rule : stage.rules)
            allRuleIds.insert(rule.Id);
    }

    for (const auto& stage : stages) {
        for (const auto& rule : stage.rules) {
            for (uint32_t dep : rule.Dependencies) {
                if (allRuleIds.find(dep) == allRuleIds.end()) {
                    result.valid = false;
                    result.errors.emplace_back("Missing dependency for rule");
                }
            }
        }
    }

    return result;
}

inline PipelineValidationResult ValidateProfile(RuleProfile profile) {
    const RuleEngine engine = BuildProfile(profile);
    return ValidateEngine(engine);
}

inline PipelineValidationResult ValidateProfile(std::string_view profileName) {
    const RuleEngine engine = BuildProfile(profileName);
    return ValidateEngine(engine);
}

} // namespace BitFlow::Core::Rules
