#include <BitFlow/core/rules/RulePipeline.h>
#include <TestAssert.h>
#include <unordered_set>

using namespace BitFlow::Core::Rules;

namespace {

std::unordered_set<uint32_t> CollectRuleIds(const RuleEngine& engine) {
    std::unordered_set<uint32_t> ids;
    for (const auto& stage : engine.Stages()) {
        for (const auto& rule : stage.rules)
            ids.insert(rule.Id);
    }
    return ids;
}

bool HasRule(const std::unordered_set<uint32_t>& ids, RuleId id) {
    return ids.find(static_cast<uint32_t>(id)) != ids.end();
}

int Test_FactorizeSafe_ContainsExpected_ExcludesDistribute() {
    const RuleEngine bitwiseSafe = BuildProfile("factorize_bitwise_safe");
    const auto bitwiseIds = CollectRuleIds(bitwiseSafe);
    BF_TEST(HasRule(bitwiseIds, RuleId::Normalize_Flatten));
    BF_TEST(HasRule(bitwiseIds, RuleId::Normalize_Order));
    BF_TEST(HasRule(bitwiseIds, RuleId::Factorize_XorAnd));
    BF_TEST(HasRule(bitwiseIds, RuleId::Factorize_XorPairCancel));
    BF_TEST(!HasRule(bitwiseIds, RuleId::Factorize_Distribute));

    const RuleEngine fullSafe = BuildProfile("factorize_full_safe");
    const auto fullIds = CollectRuleIds(fullSafe);
    BF_TEST(HasRule(fullIds, RuleId::Factorize_AddLinearMultiplicity));
    BF_TEST(HasRule(fullIds, RuleId::Factorize_AddCommonFactor));
    BF_TEST(!HasRule(fullIds, RuleId::Factorize_Distribute));
    return 0;
}

int Test_ShaSafeAlias_ContainsNormalizeSimplifyAndCHMAJ() {
    const RuleEngine shaSafe = BuildProfile("sha_safe");
    const auto ids = CollectRuleIds(shaSafe);

    BF_TEST(HasRule(ids, RuleId::Normalize_Flatten));
    BF_TEST(HasRule(ids, RuleId::Normalize_Order));
    BF_TEST(HasRule(ids, RuleId::Simplify_XorCancel));
    BF_TEST(HasRule(ids, RuleId::Simplify_AddZero));
    BF_TEST(HasRule(ids, RuleId::Simplify_CH));
    BF_TEST(HasRule(ids, RuleId::Simplify_MAJ));
    BF_TEST(!HasRule(ids, RuleId::Factorize_Distribute));
    return 0;
}

int Test_ArithmeticFactorizeProfile_DependencyCompleteAndFocused() {
    const auto validation = ValidateProfile("factorize_arithmetic_safe");
    BF_TEST(validation.valid);

    const RuleEngine arithmeticSafe = BuildProfile("factorize_arithmetic_safe");
    const auto ids = CollectRuleIds(arithmeticSafe);

    BF_TEST(HasRule(ids, RuleId::Normalize_Flatten));
    BF_TEST(HasRule(ids, RuleId::Normalize_Order));
    BF_TEST(HasRule(ids, RuleId::Factorize_AddLinearMultiplicity));
    BF_TEST(HasRule(ids, RuleId::Factorize_AddCommonFactor));
    BF_TEST(HasRule(ids, RuleId::Factorize_MulCombineConstants));

    BF_TEST(!HasRule(ids, RuleId::Factorize_XorAnd));
    BF_TEST(!HasRule(ids, RuleId::Factorize_Distribute));
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(Test_FactorizeSafe_ContainsExpected_ExcludesDistribute);
    BF_RUN_TEST(Test_ShaSafeAlias_ContainsNormalizeSimplifyAndCHMAJ);
    BF_RUN_TEST(Test_ArithmeticFactorizeProfile_DependencyCompleteAndFocused);
    return 0;
}
