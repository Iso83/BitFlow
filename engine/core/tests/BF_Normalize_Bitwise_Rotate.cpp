#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestRotateModuloBitwidth_ReducesConstantAmount() {
    MakeExprStore(32);
    const auto rule = Normalize::Bitwise::Get_Rotate_ModuloBitWidth_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    const Expr& out =
        GetExpr(Rewrite(engine, x.RotL(35), &names, PrintOptions{}.ExplicitGroups().ShowOpTypes().RotAsFunction()));

    BF_TEST(out.op == OpType::RotL);
    BF_TEST(out.inputs.size() == 2);
    BF_TEST(ERef(out.inputs[0]) == x);
    BF_TEST(IsConstantValue(ERef(out.inputs[1]), 3u));

    return 0;
}

int main() {
    BF_RUN_TEST(TestRotateModuloBitwidth_ReducesConstantAmount);
    return 0;
}
