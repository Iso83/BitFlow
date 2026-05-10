#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestXorOrdering() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());

    auto x = V("x");
    auto y = V("y");

    auto r = Rewrite(engine, y ^ x ^ y ^ x);
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::Xor);
    BF_TEST(out.inputs.size() == 4);

    BF_TEST(ERef(out.inputs[0]) == x);
    BF_TEST(ERef(out.inputs[1]) == x);
    BF_TEST(ERef(out.inputs[2]) == y);
    BF_TEST(ERef(out.inputs[3]) == y);

    return 0;
}

int main() {
    BF_RUN_TEST(TestXorOrdering);
    return 0;
}