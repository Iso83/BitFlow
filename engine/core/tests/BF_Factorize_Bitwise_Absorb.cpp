#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestAndAbsorb() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_AndAbsorb_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto inner = a | b;

    BF_SAFE_REWRITE(r, Rewrite(engine, a & inner));

    BF_TEST(r == a);
    return 0;
}

int TestOrAbsorb() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_OrAbsorb_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto inner = a & b;

    BF_SAFE_REWRITE(r, Rewrite(engine, a | inner));

    BF_TEST(r == a);
    return 0;
}

int main() {
    BF_RUN_TEST(TestAndAbsorb);
    BF_RUN_TEST(TestOrAbsorb);
    return 0;
}