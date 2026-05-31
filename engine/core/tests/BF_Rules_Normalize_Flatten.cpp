#include <ExprTestUtils.h>
#include <RuleTestUtils.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestXorFlatten() {
    MakeExprStore(32);
    const auto rule = Normalize::Get_Flatten_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");
    auto z = V("z");

    BF_SAFE_REWRITE(r, BF_REWRITE((x ^ y) ^ z));

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(InputSize(r) == 3);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == y; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == z; }));

    return 0;
}

int TestNotNotDoesNotFlatten() {
    MakeExprStore(32);
    const auto rule = Normalize::Get_Flatten_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, BF_REWRITE(-(-x)));

    BF_TEST(Op(r) == OpType::Neg);
    BF_TEST(InputSize(r) == 1);

    auto inner = Input(r, 0);

    BF_TEST(Op(inner) == OpType::Neg);
    BF_TEST(InputSize(inner) == 1);
    BF_TEST(Input(inner, 0) == x);

    return 0;
}

int main() {
    BF_RUN_TEST(TestXorFlatten);
    BF_RUN_TEST(TestNotNotDoesNotFlatten);
    return 0;
}
