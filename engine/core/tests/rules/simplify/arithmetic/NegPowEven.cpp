#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

#include <BitFlow/engine/core/rules/RulePipeline.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestNegPowEven_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_NegPowEven_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    BF_SAFE_REWRITE(r, BF_REWRITE((-x).Pow(2)));

    CPPTEST_ASSERT(IsPow(r, x, 2u));
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

    CPPTEST_ASSERT(Op(r) == OpType::Pow);
    CPPTEST_ASSERT(EqualChunkValue(Input(r, 1), 2u));

    ExprRef base = Input(r, 0);

    CPPTEST_ASSERT(Op(base) == OpType::Add);

    CPPTEST_ASSERT(AnyInput(base, [&](ExprRef x) { return x == a; }));
    CPPTEST_ASSERT(AnyInput(base, [&](ExprRef x) { return x == b; }));
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

    CPPTEST_ASSERT(IsPow(r, x, 0u));
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

    CPPTEST_ASSERT(r == expr);
    return 0;
}

int main() {
    CPPTEST_RUN(TestNegPowEven_Basic);
    CPPTEST_RUN(TestNegPowEven_NestedBase);
    CPPTEST_RUN(TestNegPowEven_ZeroExponent);
    CPPTEST_RUN(TestNegPowEven_OddExponentNoRewrite);
    return 0;
}
