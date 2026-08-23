#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

#include <BitFlow/engine/core/rules/RulePipeline.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestDivFractionDenominator_Basic() {
    MakeExprStore(32);

    const auto rule = Factorize::Arithmetic::Get_DivFractionDenominator_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE(a / (b / c)));

    CPPTEST_ASSERT(Op(r) == OpType::Div);
    CPPTEST_ASSERT(InputSize(r) == 2);

    CPPTEST_ASSERT(Op(Input(r, 0)) == OpType::Mul);
    CPPTEST_ASSERT(AnyInput(Input(r, 0), [&](ExprRef in) { return in == a; }));
    CPPTEST_ASSERT(AnyInput(Input(r, 0), [&](ExprRef in) { return in == c; }));

    CPPTEST_ASSERT(Input(r, 1) == b);

    return 0;
}

int TestDivFractionDenominator_RhsNotFraction() {
    MakeExprStore(32);

    const auto rule = Factorize::Arithmetic::Get_DivFractionDenominator_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE(a / b));

    CPPTEST_ASSERT(Op(r) == OpType::Div);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(Input(r, 0) == a);
    CPPTEST_ASSERT(Input(r, 1) == b);

    return 0;
}

int main() {
    CPPTEST_RUN(TestDivFractionDenominator_Basic);
    CPPTEST_RUN(TestDivFractionDenominator_RhsNotFraction);

    return 0;
}
