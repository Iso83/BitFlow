#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestAndZero() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Zero_Dominance_Rule());

    auto x = V("x");

    auto r = Rewrite(engine, (x & False()) & False());

    BF_TEST(IsFalse(r));

    return 0;
}

int TestXorZero() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Zero_Rule());

    auto x = V("x");

    BF_TEST(Rewrite(engine, False() ^ (x ^ False())) == x);

    return 0;
}

int main() {
    BF_RUN_TEST(TestAndZero);
    BF_RUN_TEST(TestXorZero);
    return 0;
}