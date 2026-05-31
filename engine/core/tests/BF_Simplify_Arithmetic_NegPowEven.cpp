#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestUtils.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestNegPowEven_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_NegPowEven_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    BF_SAFE_REWRITE(r, BF_REWRITE((-x).Pow(2)));

    BF_TEST(IsPow(r, x, 2u));
    return 0;
}

int TestNegPowEven_NestedBase() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_NegPowEven_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    BF_SAFE_REWRITE(r, BF_REWRITE((-(a + b)).Pow(2)));

    BF_TEST(Op(r) == OpType::Pow);
    BF_TEST(EqualChunkValue(Input(r, 1), 2u));

    ExprRef base = Input(r, 0);

    BF_TEST(Op(base) == OpType::Add);

    BF_TEST(AnyInput(base, [&](ExprRef x) { return x == a; }));
    BF_TEST(AnyInput(base, [&](ExprRef x) { return x == b; }));
    return 0;
}

int TestNegPowEven_ZeroExponent() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_NegPowEven_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, BF_REWRITE((-x).Pow(0)));

    BF_TEST(IsPow(r, x, 0u));
    return 0;
}

int TestNegPowEven_OddExponentNoRewrite() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_NegPowEven_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto expr = (-x).Pow(3);
    BF_SAFE_REWRITE(r, BF_REWRITE(expr));

    BF_TEST(r == expr);
    return 0;
}

int main() {
    BF_RUN_TEST(TestNegPowEven_Basic);
    BF_RUN_TEST(TestNegPowEven_NestedBase);
    BF_RUN_TEST(TestNegPowEven_ZeroExponent);
    BF_RUN_TEST(TestNegPowEven_OddExponentNoRewrite);
    return 0;
}
