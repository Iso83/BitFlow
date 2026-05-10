#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

static RuleEngine MakeEngine() {

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.AddRule(Factorize::Bitwise::Get_And_Absorb_Rule());
    return engine;
}

int TestAndAbsorb() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();
    auto a = V("a");
    auto b = V("b");
    auto inner = a | b;

    BF_TEST(Rewrite(engine, a & inner) == a);
    return 0;
}

int TestOrAbsorb() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();
    auto a = V("a");
    auto b = V("b");
    auto inner = a & b;

    BF_TEST(Rewrite(engine, a | inner) == a);
    return 0;
}

int main() {
    BF_RUN_TEST(TestAndAbsorb);
    BF_RUN_TEST(TestOrAbsorb);
    return 0;
}