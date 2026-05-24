#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestDivFractionDenominator_Basic() {
    MakeExprStore(32);

    const auto rule = Factorize::Arithmetic::Get_DivFractionDenominator_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, Rewrite(engine, a / (b / c)));

    BF_TEST(Op(r) == OpType::Div);
    BF_TEST(InputSize(r) == 2);

    BF_TEST(Op(Input(r, 0)) == OpType::Mul);
    BF_TEST(AnyInput(Input(r, 0), [&](ExprRef in) { return in == a; }));
    BF_TEST(AnyInput(Input(r, 0), [&](ExprRef in) { return in == c; }));

    BF_TEST(Input(r, 1) == b);

    return 0;
}

int TestDivFractionDenominator_RhsNotFraction() {
    MakeExprStore(32);

    const auto rule = Factorize::Arithmetic::Get_DivFractionDenominator_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, Rewrite(engine, a / b));

    BF_TEST(Op(r) == OpType::Div);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(Input(r, 0) == a);
    BF_TEST(Input(r, 1) == b);

    return 0;
}

int main() {
    BF_RUN_TEST(TestDivFractionDenominator_Basic);
    BF_RUN_TEST(TestDivFractionDenominator_RhsNotFraction);

    return 0;
}
