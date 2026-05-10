#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestXorDedup() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyBitwise());

    auto x = V("x");
    auto y = V("y");

    auto r1 = Rewrite(engine, x ^ y);
    auto r2 = Rewrite(engine, y ^ x);

    BF_TEST(r1 == r2);

    auto exprR1 = GetExpr(r1);
    BF_TEST(exprR1.op == OpType::Xor);
    BF_TEST(exprR1.inputs.size() == 2);
    BF_TEST(exprR1.inputs[0].value() < exprR1.inputs[1].value());
    return 0;
}

int main() {
    BF_RUN_TEST(TestXorDedup);
    return 0;
}