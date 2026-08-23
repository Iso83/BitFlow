#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

#include <BitFlow/engine/core/rules/RulePipeline.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int Test_SubNeg_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubNeg_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_NegNeg_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    const auto a = V("a");
    const auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE(a - (-b)));

    CPPTEST_ASSERT(Op(r) == OpType::Add);
    CPPTEST_ASSERT(InputSize(r) == 2);

    CPPTEST_ASSERT((Input(r, 0) == a && Input(r, 1) == b) || (Input(r, 0) == b && Input(r, 1) == a));
    return 0;
}

int Test_SubNeg_ConstantFold() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubNeg_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_NegNeg_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    BF_SAFE_REWRITE(r, BF_REWRITE(C(5) - (-C(7))));

    CPPTEST_ASSERT(EqualChunkValue(r, 12u));
    return 0;
}

int main() {
    CPPTEST_RUN(Test_SubNeg_Basic);
    CPPTEST_RUN(Test_SubNeg_ConstantFold);
    return 0;
}
