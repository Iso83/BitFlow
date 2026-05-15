#include <BitFlow/core/rules/RulePipeline.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Rules;

static bool ValidatePipelineContainsDependencies(const RuleEngine& engine, const Rule& rule) {
    const auto result = engine.AnalyzeDependencies(rule);

    if (result.missing.empty())
        return true;

    std::cout << "\n=== Pipeline Dependency Validation ===\n";
    std::cout << "Rule: " << result.rule.value << "\n";

    std::cout << "\nMissing:\n";
    for (const auto& key : result.missing)
        std::cout << " - " << key.value << "\n";

    if (!result.extra.empty()) {
        std::cout << "\nIgnored extra:\n";
        for (const auto& key : result.extra)
            std::cout << " - " << key.value << "\n";
    }

    std::cout << std::endl;
    return false;
}

#define BF_VALIDATE_PIPELINE_HAS_DEPS(engine, rule) BF_TEST(ValidatePipelineContainsDependencies(engine, rule))

int TestBuildNormalize_ValidateDependencies() {
    auto engine = BuildNormalize();

    BF_VALIDATE_PIPELINE_HAS_DEPS(engine, Normalize::Get_Order_Rule());
    return 0;
}

int TestBuildSimplifyBitwise_ValidateDependencies() {
    auto engine = BuildSimplifyBitwise();

    BF_VALIDATE_PIPELINE_HAS_DEPS(engine, Simplify::Bitwise::Get_XorAndReduction_Rule());
    return 0;
}

int TestBuildSimplifyFull_ValidateDependencies() {
    auto engine = BuildSimplifyFull();

    BF_VALIDATE_PIPELINE_HAS_DEPS(engine, Simplify::Arithmetic::Get_AddZero_Rule());
    BF_VALIDATE_PIPELINE_HAS_DEPS(engine, Simplify::Bitwise::Get_XorCancel_Rule());

    return 0;
}

int TestBuildExplore_ValidateDependencies() {
    auto engine = BuildExplore();

    BF_VALIDATE_PIPELINE_HAS_DEPS(engine, Factorize::Bitwise::Get_XorAnd_Rule());
    BF_VALIDATE_PIPELINE_HAS_DEPS(engine, Factorize::Arithmetic::Get_AddCommonFactor_Rule());

    return 0;
}

int TestRulePipeline_NoRedundantDirectDependencies() {

    RuleEngine engine;

    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyArithmetic());
    engine.Merge(BuildSimplifyBitwise());
    engine.Merge(BuildFactorizeArithmetic());
    engine.Merge(BuildFactorizeBitwise());

    bool failed = false;

    for (const auto& rule : engine.Rules()) {

        auto result = engine.AnalyzeDependencies(rule);

        if (result.redundant.empty())
            continue;

        failed = true;

        PrintDependencyValidation(result);
    }

    BF_TEST(!failed);

    return 0;
}

int main() {
    BF_RUN_TEST(TestBuildNormalize_ValidateDependencies);
    BF_RUN_TEST(TestBuildSimplifyBitwise_ValidateDependencies);
    BF_RUN_TEST(TestBuildSimplifyFull_ValidateDependencies);
    BF_RUN_TEST(TestBuildExplore_ValidateDependencies);
    BF_RUN_TEST(TestRulePipeline_NoRedundantDirectDependencies);
    return 0;
}