#include <ExprTestUtils.h>
#include <RuleTestUtils.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestAndFold() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_AndFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    {
        BF_SAFE_REWRITE(r, BF_REWRITE(x & True() & True()));
        BF_TEST(r == x);
    }

    {
        BF_SAFE_REWRITE(r, BF_REWRITE(x & False() & True()));
        BF_TEST(IsFalse(r));
    }

    return 0;
}

int TestOrFold() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_OrFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    {
        BF_SAFE_REWRITE(r, BF_REWRITE(x | False() | False()));
        BF_TEST(r == x);
    }

    {
        BF_SAFE_REWRITE(r, BF_REWRITE(x | True() | False()));
        BF_TEST(IsTrue(r));
    }

    return 0;
}

int TestXorFold() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, BF_REWRITE(x ^ True() ^ True()));

    BF_TEST(r == x);
    return 0;
}

int TestXorFoldAllConstZero() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    BF_SAFE_REWRITE(r, BF_REWRITE(True() ^ True()));

    BF_TEST(IsFalse(r));

    return 0;
}

int main() {
    BF_RUN_TEST(TestAndFold);
    BF_RUN_TEST(TestOrFold);
    BF_RUN_TEST(TestXorFold);
    BF_RUN_TEST(TestXorFoldAllConstZero);
    return 0;
}