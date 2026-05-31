#include <ExprTestUtils.h>
#include <RuleTestUtils.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestFactorize_CombineNestedMulConstants() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_MulCombineConstants_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Factorize::Arithmetic::Get_AddLinearMultiplicity_Rule());
    engine.AddRule(Factorize::Arithmetic::Get_AddCommonFactor_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    BF_SAFE_REWRITE(r, BF_REWRITE(C(3) * (a * 2)));

    BF_TEST(Op(r) == OpType::Mul);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 6u); }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    return 0;
}

int TestSimplify_CombineMulConstants_NonAdjacent() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_MulCombineConstants_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Factorize::Arithmetic::Get_AddLinearMultiplicity_Rule());
    engine.AddRule(Factorize::Arithmetic::Get_AddCommonFactor_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(C(2) * a * 3));

    BF_TEST(Op(r) == OpType::Mul);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(EqualChunkValue(Input(r, 0), 6u));
    BF_TEST(Input(r, 1) == a);
    return 0;
}

int main() {
    BF_RUN_TEST(TestFactorize_CombineNestedMulConstants);
    BF_RUN_TEST(TestSimplify_CombineMulConstants_NonAdjacent);
    return 0;
}
