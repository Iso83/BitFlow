#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestMulOne_Nested() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_MulOne_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, BF_REWRITE((x * 1) * 1));

    CPPTEST_ASSERT(r == x);
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

    BF_SAFE_REWRITE(r, BF_REWRITE(C(1) * 1 * 1));

    CPPTEST_ASSERT(EqualChunkValue(r, 1u));
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
    BF_SAFE_REWRITE(r, BF_REWRITE(y * 1 * x));

    CPPTEST_ASSERT(Op(r) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(Input(r, 0) == x);
    CPPTEST_ASSERT(Input(r, 1) == y);
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

    BF_SAFE_REWRITE(r, BF_REWRITE(x / 1));

    CPPTEST_ASSERT(r == x);
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

    BF_SAFE_REWRITE(r, BF_REWRITE(x.Pow(1)));

    CPPTEST_ASSERT(r == x);
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

    BF_SAFE_REWRITE(r, BF_REWRITE(expr));

    CPPTEST_ASSERT(r == expr);
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

    BF_SAFE_REWRITE(r, BF_REWRITE(expr));

    CPPTEST_ASSERT(r == expr);
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

    BF_SAFE_REWRITE(r, BF_REWRITE(x / x));

    CPPTEST_ASSERT(EqualChunkValue(r, 1u));
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

    BF_SAFE_REWRITE(r, BF_REWRITE(expr));

    CPPTEST_ASSERT(r == expr);
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

    BF_SAFE_REWRITE(r, BF_REWRITE(x % 1));

    CPPTEST_ASSERT(EqualChunkValue(r, 0u));
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

    BF_SAFE_REWRITE(r, BF_REWRITE(expr));

    CPPTEST_ASSERT(r == expr);
    return 0;
}

int main() {
    CPPTEST_RUN(TestMulOne_Nested);
    CPPTEST_RUN(TestMulOne_AllOnesBecomeConstOne);
    CPPTEST_RUN(TestMulOne_CanonicalOrderRegression);
    CPPTEST_RUN(TestDivOne_Basic);
    CPPTEST_RUN(TestPowOne_Basic);
    CPPTEST_RUN(TestPowOne_GuardExponentTwoStaysPow);
    CPPTEST_RUN(TestDivOne_GuardLeftOneStaysDiv);
    CPPTEST_RUN(TestDivSelf_Basic);
    CPPTEST_RUN(TestDivSelf_DifferentInputsStayDiv);
    CPPTEST_RUN(TestModOne_Basic);
    CPPTEST_RUN(TestModOne_GuardDifferentDivisorStaysMod);
    return 0;
}
