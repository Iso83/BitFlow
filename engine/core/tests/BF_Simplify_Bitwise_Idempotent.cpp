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
    engine.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());
    return engine;
}

int TestAndIdempotent() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();

    auto a = V("a");

    BF_TEST(Rewrite(engine, a & a) == a);

    return 0;
}

int TestAndIdempotentMixed() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();

    auto a = V("a");
    auto b = V("b");

    auto r = Rewrite(engine, a & b & a);
    auto out = ExprOf(r);

    BF_TEST(out.op == OpType::And);
    BF_TEST(out.inputs.size() == 2);

    BF_TEST(ERef(out.inputs[0]) == a);
    BF_TEST(ERef(out.inputs[1]) == b);

    return 0;
}

int TestOrIdempotent() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();

    auto a = V("a");

    BF_TEST(Rewrite(engine, a | a) == a);

    return 0;
}

int TestOrIdempotentMixed() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();

    auto a = V("a");
    auto b = V("b");

    auto r = Rewrite(engine, b | a | b);
    auto out = ExprOf(r);

    BF_TEST(out.op == OpType::Or);
    BF_TEST(out.inputs.size() == 2);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == b; }));

    return 0;
}

int main() {
    BF_RUN_TEST(TestAndIdempotent);
    BF_RUN_TEST(TestAndIdempotentMixed);
    BF_RUN_TEST(TestOrIdempotent);
    BF_RUN_TEST(TestOrIdempotentMixed);
    return 0;
}