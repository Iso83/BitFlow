#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

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
    CPPTEST_ASSERT(Op(r) == OpType::Pow);
    CPPTEST_ASSERT(ExprOf(Input(r, 1)).knownValue == 2);
    CPPTEST_ASSERT(Op(Input(r, 0)) == OpType::Add);
    CPPTEST_ASSERT(AnyInput(Input(r, 0), [&](ExprRef in) { return in == a; }));
    CPPTEST_ASSERT(AnyInput(Input(r, 0), [&](ExprRef in) { return in == b; }));
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
    CPPTEST_ASSERT(Op(r) == OpType::Pow);
    CPPTEST_ASSERT(Op(Input(r, 0)) == OpType::Sub);

    ExprRef sub = Input(r, 0);

    CPPTEST_ASSERT(AnyInput(sub, [&](ExprRef in) { return in == a; }));
    CPPTEST_ASSERT(AnyInput(sub, [&](ExprRef in) { return in == b; }));
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
    CPPTEST_ASSERT(Op(r1) == OpType::Pow);

    CPPTEST_ASSERT(EqualChunkValue(Input(r1, 1), 2u));

    ExprRef add = Input(r1, 0);

    CPPTEST_ASSERT(AnyInput(add, [&](ExprRef x) { return x == a; }));
    CPPTEST_ASSERT(AnyInput(add, [&](ExprRef x) { return Op(x) == OpType::Const && ExprOf(x).knownValue == 1; }));

    BF_SAFE_REWRITE(r2, BF_REWRITE(a.Pow(C(2)) - (C(6) * a) + C(9)));
    CPPTEST_ASSERT(Op(r2) == OpType::Pow);

    CPPTEST_ASSERT(EqualChunkValue(Input(r2, 1), 2u));

    ExprRef sub = Input(r2, 0);

    CPPTEST_ASSERT(AnyInput(sub, [&](ExprRef x) { return x == a; }));

    CPPTEST_ASSERT(AnyInput(sub, [&](ExprRef x) { return Op(x) == OpType::Const && ExprOf(x).knownValue == 3; }));

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
    CPPTEST_ASSERT(r1 != (a + C(3)).Pow(C(2)));

    BF_SAFE_REWRITE(r2, BF_REWRITE(a.Pow(C(2)) + (C(2) * a * b) + c.Pow(C(2))));
    CPPTEST_ASSERT(r2 != (a + b).Pow(C(2)));
    return 0;
}

int main() {
    CPPTEST_RUN(TestPerfectSquare_PositiveAB);
    CPPTEST_RUN(TestPerfectSquare_NegativeAB);
    CPPTEST_RUN(TestPerfectSquare_LinearAndConstant);
    CPPTEST_RUN(TestPerfectSquare_NoMatch);
    return 0;
}
