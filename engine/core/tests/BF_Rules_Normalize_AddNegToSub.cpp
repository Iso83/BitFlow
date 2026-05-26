#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestAddNegToSub_Basic() {
    MakeExprStore(32);
    const auto rule = Normalize::Arithmetic::Get_AddNegToSub_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");
    BF_SAFE_REWRITE(r, Rewrite(engine, x + (-y)));

    BF_TEST(Op(r) == OpType::Sub);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(Input(r, 0) == x);
    BF_TEST(Input(r, 1) == y);
    return 0;
}

int TestAddNegToSub_NoMatch_MultipleNegatives() {
    MakeExprStore(32);
    const auto rule = Normalize::Arithmetic::Get_AddNegToSub_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, Rewrite(engine, a + b + (-c)));

    BF_TEST(InputSize(r) == 3);

    BF_TEST(AnyInput(r, [&](ExprRef x) { return x == a; }));
    BF_TEST(AnyInput(r, [&](ExprRef x) { return x == b; }));

    BF_TEST(AnyInput(r, [&](ExprRef x) { return Op(x) == OpType::Neg && Input(x, 0) == c; }));
    return 0;
}

int main() {
    BF_RUN_TEST(TestAddNegToSub_Basic);
    BF_RUN_TEST(TestAddNegToSub_NoMatch_MultipleNegatives);
    return 0;
}
