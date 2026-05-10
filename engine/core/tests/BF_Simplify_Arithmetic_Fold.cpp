#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

static RuleEngine MakeEngine() {

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Add_Fold_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Add_Zero_Rule());
    return engine;
}

int TestAddFold() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();

    auto x = V("x");

    auto r = Rewrite(engine, x + 10 + 20);
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::Add);
    BF_TEST(out.inputs.size() == 2);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return IsConstantValue(in, 30u); }));

    return 0;
}

int main() {
    BF_RUN_TEST(TestAddFold);
    return 0;
}