#include <ExprTestUtils.h>
#include <RuleTestUtils.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestAndComplement() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_Complement_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(a & ~a));

    BF_TEST(IsFalse(r));
    return 0;
}

int TestOrComplement() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_Complement_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(a | ~a));

    BF_TEST(IsTrue(r));
    return 0;
}

int main() {
    BF_RUN_TEST(TestAndComplement);
    BF_RUN_TEST(TestOrComplement);
    return 0;
}