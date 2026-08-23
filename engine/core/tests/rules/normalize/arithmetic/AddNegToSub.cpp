#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

#include <BitFlow/engine/core/rules/RulePipeline.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestAddNegToSub_Basic() {
    MakeExprStore(32);
    const auto rule = Normalize::Arithmetic::Get_AddNegToSub_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");
    BF_SAFE_REWRITE(r, BF_REWRITE(x + (-y)));

    CPPTEST_ASSERT(Op(r) == OpType::Sub);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(Input(r, 0) == x);
    CPPTEST_ASSERT(Input(r, 1) == y);
    return 0;
}

int TestAddNegToSub_NoMatch_MultipleNegatives() {
    MakeExprStore(32);
    const auto rule = Normalize::Arithmetic::Get_AddNegToSub_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE(a + b + (-c)));

    CPPTEST_ASSERT(InputSize(r) == 3);

    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef x) { return x == a; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef x) { return x == b; }));

    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef x) { return Op(x) == OpType::Neg && Input(x, 0) == c; }));
    return 0;
}

int main() {
    CPPTEST_RUN(TestAddNegToSub_Basic);
    CPPTEST_RUN(TestAddNegToSub_NoMatch_MultipleNegatives);
    return 0;
}
