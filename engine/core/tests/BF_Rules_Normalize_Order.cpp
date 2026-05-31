#include <ExprTestUtils.h>
#include <RuleTestUtils.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestXorOrdering() {
    MakeExprStore(32);
    const auto rule = Normalize::Get_Order_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r, BF_REWRITE(y ^ x ^ y ^ x));

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