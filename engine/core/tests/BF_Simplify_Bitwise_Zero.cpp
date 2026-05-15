#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestAndZero() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_AndZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, (x & False()) & False()));

    BF_TEST(IsFalse(r));
    return 0;
}

int TestOrZero() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_OrZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, (x | False()) | False()));

    BF_TEST(r == x);
    return 0;
}

int TestXorZero() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, False() ^ (x ^ False())));

    BF_TEST(r == x);
    return 0;
}

int main() {
    BF_RUN_TEST(TestAndZero);
    BF_RUN_TEST(TestOrZero);
    BF_RUN_TEST(TestXorZero);
    return 0;
}