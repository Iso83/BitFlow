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
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    return engine;
}

int TestAndXorReduction_RightXor() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();
    auto x = V("x");
    auto y = V("y");
    auto r = Rewrite(engine, x & (x ^ y));

    BF_TEST(Op(r) == OpType::And);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == y; }));

    return 0;
}

int TestAndXorReduction_MultiArgAnd() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();
    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto r = Rewrite(engine, c & (a ^ c) & (b ^ c));

    BF_TEST(Op(r) == OpType::And);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == c; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == a; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == b; }));
    BF_TEST(!AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Xor; }));

    return 0;
}

int main() {
    BF_RUN_TEST(TestAndXorReduction_RightXor);
    BF_RUN_TEST(TestAndXorReduction_MultiArgAnd);
    return 0;
}
