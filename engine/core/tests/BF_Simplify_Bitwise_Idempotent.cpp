#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestAndIdempotent() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_Idempotent_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, Rewrite(engine, a & a));
    BF_TEST(r == a);

    return 0;
}

int TestAndIdempotentMixed() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_Idempotent_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, Rewrite(engine, a & b & a));

    BF_TEST(Op(r) == OpType::And);
    BF_TEST(InputSize(r) == 2);

    BF_TEST(Input(r, 0) == a);
    BF_TEST(Input(r, 1) == b);

    return 0;
}

int TestOrIdempotent() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_Idempotent_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, Rewrite(engine, a | a));

    BF_TEST(r == a);

    return 0;
}

int TestOrIdempotentMixed() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_Idempotent_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, Rewrite(engine, b | a | b));

    BF_TEST(Op(r) == OpType::Or);
    BF_TEST(InputSize(r) == 2);

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