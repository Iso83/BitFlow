#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

#include <BitFlow/engine/core/rules/RulePipeline.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestMulFractionNumerator_FractionOnRhs() {
    MakeExprStore(32);

    const auto rule = Factorize::Arithmetic::Get_MulFractionNumerator_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    BF_SAFE_REWRITE(r, BF_REWRITE(C(2) * (C(3) / C(8))));

    CPPTEST_ASSERT(Op(r) == OpType::Div);
    CPPTEST_ASSERT(InputSize(r) == 2);

    CPPTEST_ASSERT(EqualChunkValue(Input(r, 1), 8u));

    CPPTEST_ASSERT(Op(Input(r, 0)) == OpType::Mul);

    CPPTEST_ASSERT(AnyInput(Input(r, 0), [](ExprRef in) { return EqualChunkValue(in, 2u); }));

    CPPTEST_ASSERT(AnyInput(Input(r, 0), [](ExprRef in) { return EqualChunkValue(in, 3u); }));

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

    BF_SAFE_REWRITE(r, BF_REWRITE((a / b) * c));

    CPPTEST_ASSERT(Op(r) == OpType::Div);
    CPPTEST_ASSERT(InputSize(r) == 2);

    CPPTEST_ASSERT(Input(r, 1) == b);

    CPPTEST_ASSERT(Op(Input(r, 0)) == OpType::Mul);

    CPPTEST_ASSERT(AnyInput(Input(r, 0), [&](ExprRef in) { return in == a; }));

    CPPTEST_ASSERT(AnyInput(Input(r, 0), [&](ExprRef in) { return in == c; }));

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

    BF_SAFE_REWRITE(r, BF_REWRITE((a / b) * (c / d)));

    CPPTEST_ASSERT(Op(r) == OpType::Mul);

    return 0;
}

int main() {
    CPPTEST_RUN(TestMulFractionNumerator_FractionOnRhs);
    CPPTEST_RUN(TestMulFractionNumerator_FractionOnLhs);
    CPPTEST_RUN(TestMulFractionNumerator_BothFractions);

    return 0;
}
