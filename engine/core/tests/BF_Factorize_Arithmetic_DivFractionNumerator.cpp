#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestDivFractionNumerator_Basic() {
    MakeExprStore(32);

    const auto rule = Factorize::Arithmetic::Get_DivFractionNumerator_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, Rewrite(engine, (a / b) / c));

    BF_TEST(Op(r) == OpType::Div);
    BF_TEST(InputSize(r) == 2);

    BF_TEST(Input(r, 0) == a);

    BF_TEST(Op(Input(r, 1)) == OpType::Mul);
    BF_TEST(AnyInput(Input(r, 1), [&](ExprRef in) { return in == b; }));
    BF_TEST(AnyInput(Input(r, 1), [&](ExprRef in) { return in == c; }));

    return 0;
}

int TestDivFractionNumerator_LhsNotFraction() {
    MakeExprStore(32);

    const auto rule = Factorize::Arithmetic::Get_DivFractionNumerator_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto c = V("c");

    BF_SAFE_REWRITE(r, Rewrite(engine, a / c));

    BF_TEST(Op(r) == OpType::Div);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(Input(r, 0) == a);
    BF_TEST(Input(r, 1) == c);

    return 0;
}

int main() {
    BF_RUN_TEST(TestDivFractionNumerator_Basic);
    BF_RUN_TEST(TestDivFractionNumerator_LhsNotFraction);

    return 0;
}
