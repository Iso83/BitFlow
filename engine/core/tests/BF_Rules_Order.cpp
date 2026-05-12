#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestXorOrdering() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());

    auto x = V("x");
    auto y = V("y");

    auto r = Rewrite(engine, y ^ x ^ y ^ x);

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(InputSize(r) == 4);

    BF_TEST(Input(r, 0) == x);
    BF_TEST(Input(r, 1) == x);
    BF_TEST(Input(r, 2) == y);
    BF_TEST(Input(r, 3) == y);

    return 0;
}

int main() {
    BF_RUN_TEST(TestXorOrdering);
    return 0;
}