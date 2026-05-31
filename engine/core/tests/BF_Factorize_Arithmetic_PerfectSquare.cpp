#include <ExprTestUtils.h>
#include <RuleTestUtils.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestPerfectSquare_PositiveAB() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_PerfectSquare_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    BF_SAFE_REWRITE(r, BF_REWRITE(a.Pow(C(2)) + (C(2) * a * b) + b.Pow(C(2))));
    BF_TEST(Op(r) == OpType::Pow);
    BF_TEST(ExprOf(Input(r, 1)).knownValue == 2);
    BF_TEST(Op(Input(r, 0)) == OpType::Add);
    BF_TEST(AnyInput(Input(r, 0), [&](ExprRef in) { return in == a; }));
    BF_TEST(AnyInput(Input(r, 0), [&](ExprRef in) { return in == b; }));
    return 0;
}

int TestPerfectSquare_NegativeAB() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_PerfectSquare_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    BF_SAFE_REWRITE(r, BF_REWRITE(a.Pow(C(2)) - (C(2) * a * b) + b.Pow(C(2))));
    BF_TEST(Op(r) == OpType::Pow);
    BF_TEST(Op(Input(r, 0)) == OpType::Sub);

    ExprRef sub = Input(r, 0);

    BF_TEST(AnyInput(sub, [&](ExprRef in) { return in == a; }));
    BF_TEST(AnyInput(sub, [&](ExprRef in) { return in == b; }));
    return 0;
}

int TestPerfectSquare_LinearAndConstant() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_PerfectSquare_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    BF_SAFE_REWRITE(r1, BF_REWRITE(a.Pow(C(2)) + (C(2) * a) + C(1)));
    BF_TEST(Op(r1) == OpType::Pow);

    BF_TEST(EqualChunkValue(Input(r1, 1), 2u));

    ExprRef add = Input(r1, 0);

    BF_TEST(AnyInput(add, [&](ExprRef x) { return x == a; }));
    BF_TEST(AnyInput(add, [&](ExprRef x) { return Op(x) == OpType::Const && ExprOf(x).knownValue == 1; }));

    BF_SAFE_REWRITE(r2, BF_REWRITE(a.Pow(C(2)) - (C(6) * a) + C(9)));
    BF_TEST(Op(r2) == OpType::Pow);

    BF_TEST(EqualChunkValue(Input(r2, 1), 2u));

    ExprRef sub = Input(r2, 0);

    BF_TEST(AnyInput(sub, [&](ExprRef x) { return x == a; }));

    BF_TEST(AnyInput(sub, [&](ExprRef x) { return Op(x) == OpType::Const && ExprOf(x).knownValue == 3; }));

    return 0;
}

int TestPerfectSquare_NoMatch() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_PerfectSquare_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r1, BF_REWRITE(a.Pow(C(2)) + (C(5) * a) + C(6)));
    BF_TEST(r1 != (a + C(3)).Pow(C(2)));

    BF_SAFE_REWRITE(r2, BF_REWRITE(a.Pow(C(2)) + (C(2) * a * b) + c.Pow(C(2))));
    BF_TEST(r2 != (a + b).Pow(C(2)));
    return 0;
}

int main() {
    BF_RUN_TEST(TestPerfectSquare_PositiveAB);
    BF_RUN_TEST(TestPerfectSquare_NegativeAB);
    BF_RUN_TEST(TestPerfectSquare_LinearAndConstant);
    BF_RUN_TEST(TestPerfectSquare_NoMatch);
    return 0;
}
