#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestMulFractionNumerator_FractionOnRhs() {
    MakeExprStore(32);

    const auto rule = Factorize::Arithmetic::Get_MulFractionNumerator_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    BF_SAFE_REWRITE(r, Rewrite(engine, C(2) * (C(3) / C(8))));

    BF_TEST(Op(r) == OpType::Div);
    BF_TEST(InputSize(r) == 2);

    BF_TEST(EqualChunkValue(Input(r, 1), 8u));

    BF_TEST(Op(Input(r, 0)) == OpType::Mul);

    BF_TEST(AnyInput(Input(r, 0), [](ExprRef in) { return EqualChunkValue(in, 2u); }));

    BF_TEST(AnyInput(Input(r, 0), [](ExprRef in) { return EqualChunkValue(in, 3u); }));

    return 0;
}

int TestMulFractionNumerator_FractionOnLhs() {
    MakeExprStore(32);

    const auto rule = Factorize::Arithmetic::Get_MulFractionNumerator_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, Rewrite(engine, (a / b) * c));

    BF_TEST(Op(r) == OpType::Div);
    BF_TEST(InputSize(r) == 2);

    BF_TEST(Input(r, 1) == b);

    BF_TEST(Op(Input(r, 0)) == OpType::Mul);

    BF_TEST(AnyInput(Input(r, 0), [&](ExprRef in) { return in == a; }));

    BF_TEST(AnyInput(Input(r, 0), [&](ExprRef in) { return in == c; }));

    return 0;
}

int TestMulFractionNumerator_BothFractions() {
    MakeExprStore(32);

    const auto rule = Factorize::Arithmetic::Get_MulFractionNumerator_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");

    BF_SAFE_REWRITE(r, Rewrite(engine, (a / b) * (c / d)));

    BF_TEST(Op(r) == OpType::Mul);

    return 0;
}

int main() {
    BF_RUN_TEST(TestMulFractionNumerator_FractionOnRhs);
    BF_RUN_TEST(TestMulFractionNumerator_FractionOnLhs);
    BF_RUN_TEST(TestMulFractionNumerator_BothFractions);

    return 0;
}