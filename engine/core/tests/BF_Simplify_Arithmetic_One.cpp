#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestMulOne_Nested() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_MulOne_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, (x * 1) * 1));

    BF_TEST(r == x);
    return 0;
}

int TestMulOne_AllOnesBecomeConstOne() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_MulOne_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    BF_SAFE_REWRITE(r, Rewrite(engine, C(1) * 1 * 1));

    BF_TEST(EqualChunkValue(r, 1u));
    return 0;
}

int TestMulOne_CanonicalOrderRegression() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_MulOne_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");
    BF_SAFE_REWRITE(r, Rewrite(engine, y * 1 * x));

    BF_TEST(Op(r) == OpType::Mul);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(Input(r, 0) == x);
    BF_TEST(Input(r, 1) == y);
    return 0;
}

int TestDivOne_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_DivOne_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x / 1));

    BF_TEST(r == x);
    return 0;
}

int TestPowOne_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_PowOne_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x.Pow(1)));

    BF_TEST(r == x);
    return 0;
}

int TestPowOne_GuardExponentTwoStaysPow() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_PowOne_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto expr = x.Pow(2);

    BF_SAFE_REWRITE(r, Rewrite(engine, expr));

    BF_TEST(r == expr);
    return 0;
}

int TestDivOne_GuardLeftOneStaysDiv() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_DivOne_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto expr = C(1) / x;

    BF_SAFE_REWRITE(r, Rewrite(engine, expr));

    BF_TEST(r == expr);
    return 0;
}

int TestDivSelf_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_DivSelf_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x / x));

    BF_TEST(EqualChunkValue(r, 1u));
    return 0;
}

int TestDivSelf_DifferentInputsStayDiv() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_DivSelf_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");
    auto expr = x / y;

    BF_SAFE_REWRITE(r, Rewrite(engine, expr));

    BF_TEST(r == expr);
    return 0;
}

int TestModOne_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_ModOne_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x % 1));

    BF_TEST(EqualChunkValue(r, 0u));
    return 0;
}

int TestModOne_GuardDifferentDivisorStaysMod() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_ModOne_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto expr = x % 2;

    BF_SAFE_REWRITE(r, Rewrite(engine, expr));

    BF_TEST(r == expr);
    return 0;
}

int main() {
    BF_RUN_TEST(TestMulOne_Nested);
    BF_RUN_TEST(TestMulOne_AllOnesBecomeConstOne);
    BF_RUN_TEST(TestMulOne_CanonicalOrderRegression);
    BF_RUN_TEST(TestDivOne_Basic);
    BF_RUN_TEST(TestPowOne_Basic);
    BF_RUN_TEST(TestPowOne_GuardExponentTwoStaysPow);
    BF_RUN_TEST(TestDivOne_GuardLeftOneStaysDiv);
    BF_RUN_TEST(TestDivSelf_Basic);
    BF_RUN_TEST(TestDivSelf_DifferentInputsStayDiv);
    BF_RUN_TEST(TestModOne_Basic);
    BF_RUN_TEST(TestModOne_GuardDifferentDivisorStaysMod);
    return 0;
}
