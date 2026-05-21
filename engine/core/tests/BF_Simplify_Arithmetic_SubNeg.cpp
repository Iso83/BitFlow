#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

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

    BF_SAFE_REWRITE(r, Rewrite(engine, a - (-b)));

    BF_TEST(Op(r) == OpType::Add);
    BF_TEST(InputSize(r) == 2);

    BF_TEST((Input(r, 0) == a && Input(r, 1) == b) || (Input(r, 0) == b && Input(r, 1) == a));
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

    BF_SAFE_REWRITE(r, Rewrite(engine, C(5) - (-C(7))));

    BF_TEST(EqualChunkValue(r, 12u));
    return 0;
}

int main() {
    BF_RUN_TEST(Test_SubNeg_Basic);
    BF_RUN_TEST(Test_SubNeg_ConstantFold);
    return 0;
}