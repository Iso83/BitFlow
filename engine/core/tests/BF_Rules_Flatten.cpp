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
    return engine;
}

int TestXorFlatten() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();

    auto x = V("x");
    auto y = V("y");
    auto z = V("z");

    auto r = Rewrite(engine, (x ^ y) ^ z);

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(InputSize(r) == 3);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == y; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == z; }));

    return 0;
}

int TestNotNotDoesNotFlatten() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();

    auto x = V("x");

    auto r = Rewrite(engine, -(-x)); // of ~(~x) indien bitwise-not

    BF_TEST(Op(r) == OpType::Neg); // of Not
    BF_TEST(InputSize(r) == 1);

    auto inner = Input(r, 0);

    BF_TEST(Op(inner) == OpType::Neg); // of Not
    BF_TEST(InputSize(inner) == 1);
    BF_TEST(Input(inner, 0) == x);

    return 0;
}

int main() {
    BF_RUN_TEST(TestXorFlatten);
    BF_RUN_TEST(TestNotNotDoesNotFlatten);
    return 0;
}
