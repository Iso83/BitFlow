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

bool HasRule(const std::unordered_set<uint32_t>& ids, CoreRuleType id) {
    return ids.find(static_cast<uint32_t>(id)) != ids.end();
}

int Test_FactorizeSafe_ContainsExpected_ExcludesDistribute() {
    const RuleEngine bitwiseSafe = BuildProfile("factorize_bitwise_safe");
    const auto bitwiseIds = CollectRuleIds(bitwiseSafe);
    CoreRuleTypeBF_TEST(HasRule(bitwiseIds, CoreRuleType::Normalize_Flatten));
    CoreRuleTypeBF_TEST(HasRule(bitwiseIds, CoreRuleType::Normalize_Order));
    CoreRuleTypeBF_TEST(HasRule(bitwiseIds, CoreRuleType::Factorize_XorAnd));
    CoreRuleTypeBF_TEST(HasRule(bitwiseIds, CoreRuleType::Factorize_XorPairCancel));
    CoreRuleTypeBF_TEST(!HasRule(bitwiseIds, CoreRuleType::Factorize_Distribute));

    const RuleEngine fullSafe = BuildProfile("factorize_full_safe");
    const auto fullIds = CollectRuleIds(fullSafe);
    CoreRuleTypeBF_TEST(HasRule(fullIds, CoreRuleType::Factorize_AddLinearMultiplicity));
    CoreRuleTypeBF_TEST(HasRule(fullIds, CoreRuleType::Factorize_AddCommonFactor));
    CoreRuleTypeBF_TEST(!HasRule(fullIds, CoreRuleType::Factorize_Distribute));
    return 0;
}

int Test_ShaSafeAlias_ContainsNormalizeSimplifyAndCHMAJ() {
    const RuleEngine shaSafe = BuildProfile("sha_safe");
    const auto ids = CollectRuleIds(shaSafe);

    CoreRuleTypeBF_TEST(HasRule(ids, CoreRuleType::Normalize_Flatten));
    CoreRuleTypeBF_TEST(HasRule(ids, CoreRuleType::Normalize_Order));
    CoreRuleTypeBF_TEST(HasRule(ids, CoreRuleType::Simplify_XorCancel));
    CoreRuleTypeBF_TEST(HasRule(ids, CoreRuleType::Simplify_AddZero));
    CoreRuleTypeBF_TEST(HasRule(ids, CoreRuleType::Simplify_CH));
    CoreRuleTypeBF_TEST(HasRule(ids, CoreRuleType::Simplify_MAJ));
    CoreRuleTypeBF_TEST(!HasRule(ids, CoreRuleType::Factorize_Distribute));
    return 0;
}

int Test_ArithmeticFactorizeProfile_DependencyCompleteAndFocused() {
    const auto validation = ValidateProfile("factorize_arithmetic_safe");
    BF_TEST(validation.valid);

    const RuleEngine arithmeticSafe = BuildProfile("factorize_arithmetic_safe");
    const auto ids = CollectRuleIds(arithmeticSafe);

    CoreRuleTypeBF_TEST(HasRule(ids, CoreRuleType::Normalize_Flatten));
    CoreRuleTypeBF_TEST(HasRule(ids, CoreRuleType::Normalize_Order));
    CoreRuleTypeBF_TEST(HasRule(ids, CoreRuleType::Factorize_AddLinearMultiplicity));
    CoreRuleTypeBF_TEST(HasRule(ids, CoreRuleType::Factorize_AddCommonFactor));
    CoreRuleTypeBF_TEST(HasRule(ids, CoreRuleType::Factorize_MulCombineConstants));

    CoreRuleTypeBF_TEST(!HasRule(ids, CoreRuleType::Factorize_XorAnd));
    CoreRuleTypeBF_TEST(!HasRule(ids, CoreRuleType::Factorize_Distribute));
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(Test_FactorizeSafe_ContainsExpected_ExcludesDistribute);
    BF_RUN_TEST(Test_ShaSafeAlias_ContainsNormalizeSimplifyAndCHMAJ);
    BF_RUN_TEST(Test_ArithmeticFactorizeProfile_DependencyCompleteAndFocused);
    return 0;
}
