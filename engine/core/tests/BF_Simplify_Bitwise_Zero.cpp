#include <ExprTestUtils.h>
#include <RuleTestUtils.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestXorZero() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, BF_REWRITE(False() ^ (x ^ False())));

    BF_TEST(r == x);
    return 0;
}

int main() {
    BF_RUN_TEST(TestXorZero);
    return 0;
}