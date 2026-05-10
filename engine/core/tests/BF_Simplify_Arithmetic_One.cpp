#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <ExprTestUtils.h>
#include <TestAssert.h>
#include <vector>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

static RuleEngine MakeMultEngine() {
    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Mul_One_Rule());
    return engine;
}

static RuleEngine MakeDivEngine() {
    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Div_One_Rule());
    return engine;
}

int TestMulOne_Nested() {
    MakeExprStore(32);

    RuleEngine engine = MakeMultEngine();
    auto x = V("x");

    BF_TEST(Rewrite(engine, (x * 1) * 1) == x);
    return 0;
}

int TestMulOne_AllOnesBecomeConstOne() {
    MakeExprStore(32);

    RuleEngine engine = MakeMultEngine();
    auto r = Rewrite(engine, C(1) * 1 * 1);

    BF_TEST(IsConstantValue(r, 1u));
    return 0;
}

int TestMulOne_CanonicalOrderRegression() {
    MakeExprStore(32);

    RuleEngine engine = MakeMultEngine();
    auto x = V("x");
    auto y = V("y");
    auto r = Rewrite(engine, y * 1 * x);
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::Mul);
    BF_TEST(out.inputs.size() == 2);
    BF_TEST(ERef(out.inputs[0]) == x);
    BF_TEST(ERef(out.inputs[1]) == y);
    return 0;
}

int TestDivOne_Basic() {
    MakeExprStore(32);

    RuleEngine engine = MakeDivEngine();
    auto x = V("x");

    BF_TEST(Rewrite(engine, x / 1) == x);
    return 0;
}

int TestDivOne_GuardLeftOneStaysDiv() {
    MakeExprStore(32);

    RuleEngine engine = MakeDivEngine();
    auto x = V("x");
    auto expr = C(1) / x;

    BF_TEST(Rewrite(engine, expr) == expr);
    return 0;
}

int main() {
    BF_RUN_TEST(TestMulOne_Nested);
    BF_RUN_TEST(TestMulOne_AllOnesBecomeConstOne);
    BF_RUN_TEST(TestMulOne_CanonicalOrderRegression);
    BF_RUN_TEST(TestDivOne_Basic);
    BF_RUN_TEST(TestDivOne_GuardLeftOneStaysDiv);
    return 0;
}
