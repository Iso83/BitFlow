#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

#include <BitFlow/engine/core/rules/RulePipeline.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

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

#define BF_VALIDATE_PIPELINE_HAS_DEPS(engine, rule) CPPTEST_ASSERT(ValidatePipelineContainsDependencies(engine, rule))

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

    CPPTEST_ASSERT(!failed);

    return 0;
}

int TestRulePipeline_Explore() {
    MakeExprStore(32);

    RuleEngine engine = BuildExplore();
    auto x = V("x");
    BF_SAFE_REWRITE(r, BF_REWRITE(x.Pow(4) * x));
    return 0;
}

int main() {
    CPPTEST_RUN(TestBuildNormalize_ValidateDependencies);
    CPPTEST_RUN(TestBuildSimplifyBitwise_ValidateDependencies);
    CPPTEST_RUN(TestBuildSimplifyFull_ValidateDependencies);
    CPPTEST_RUN(TestBuildExplore_ValidateDependencies);
    CPPTEST_RUN(TestRulePipeline_NoRedundantDirectDependencies);
    CPPTEST_RUN(TestRulePipeline_Explore);
    return 0;
}
