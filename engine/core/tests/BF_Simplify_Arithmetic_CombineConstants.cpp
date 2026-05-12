#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestXorDedup() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyBitwise());

    auto x = V("x");
    auto y = V("y");

    auto r1 = Rewrite(engine, x ^ y);
    auto r2 = Rewrite(engine, y ^ x);

    BF_TEST(r1 == r2);
    BF_TEST(Op(r1) == OpType::Xor);
    BF_TEST(InputSize(r1) == 2);
    BF_TEST(Input(r1, 0).id.value() < Input(r1, 1).id.value());
    return 0;
}

int main() {
    BF_RUN_TEST(TestXorDedup);
    return 0;
}