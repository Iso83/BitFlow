#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

static RuleEngine MakeEngine() {
    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());
    engine.AddRule(Factorize::Bitwise::Get_Xor_Pair_Cancel_Rule());
    return engine;
}

int TestXorXorCancelPair() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();
    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto r = Rewrite(engine, (a ^ b) ^ (a ^ c));
    auto out = ExprOf(r);

    BF_TEST(out.op == OpType::Xor);
    BF_TEST(out.inputs.size() == 2);
    BF_TEST(CountExpr(r, a) == 0);
    BF_TEST(CountExpr(r, b) == 1);
    BF_TEST(CountExpr(r, c) == 1);
    return 0;
}

int TestXorXorCancelPair_MultiInputOddCommon() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();
    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");
    auto r = Rewrite(engine, (a ^ b) ^ (a ^ c) ^ (a ^ d));
    auto out = ExprOf(r);

    BF_TEST(out.op == OpType::Xor);
    BF_TEST(out.inputs.size() == 4);
    BF_TEST(CountExpr(r, a) == 1);
    BF_TEST(CountExpr(r, b) == 1);
    BF_TEST(CountExpr(r, c) == 1);
    BF_TEST(CountExpr(r, d) == 1);
    return 0;
}

int main() {
    BF_RUN_TEST(TestXorXorCancelPair);
    BF_RUN_TEST(TestXorXorCancelPair_MultiInputOddCommon);
    return 0;
}