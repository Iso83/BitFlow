#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int Test_And_ZeroDominance() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_AndZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, Rewrite(engine, a & 0));

    BF_TEST(IsFalse(r));
    return 0;
}

int Test_And_OneIdentity_Multi() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_AndOneIdentity_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, Rewrite(engine, a & True() & b));

    BF_TEST(Op(r) == OpType::And);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(Input(r, 0) == a);
    BF_TEST(Input(r, 1) == b);
    return 0;
}

int Test_Or_OneDominance() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_OrOneDominance_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    BF_SAFE_REWRITE(r, Rewrite(engine, a | True()));

    BF_TEST(IsTrue(r));
    return 0;
}

int Test_Or_ZeroIdentity_Multi() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_OrZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, Rewrite(engine, a | False() | b));

    BF_TEST(Op(r) == OpType::Or);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(Input(r, 0) == a);
    BF_TEST(Input(r, 1) == b);

    return 0;
}

int main() {
    BF_RUN_TEST(Test_And_ZeroDominance);
    BF_RUN_TEST(Test_And_OneIdentity_Multi);
    BF_RUN_TEST(Test_Or_OneDominance);
    BF_RUN_TEST(Test_Or_ZeroIdentity_Multi);
    return 0;
}