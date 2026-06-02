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
    engine.AddRule(Normalize::Get_Order_Rule());
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
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());
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

int TestOrFold_BitwiseConstants() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_OrFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    {
        BF_SAFE_REWRITE(r, BF_REWRITE(C(1) | C(2)));
        BF_TEST(EqualChunkValue(r, 3u));
    }

    {
        BF_SAFE_REWRITE(r, BF_REWRITE(C(1) | C(2) | a));
        BF_TEST(Op(r) == OpType::Or);
        BF_TEST(InputSize(r) == 2);
        BF_TEST(EqualChunkValue(Input(r, 0), 3u));
        BF_TEST(Input(r, 1) == a);
    }

    return 0;
}

int TestXorFold() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
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
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    BF_SAFE_REWRITE(r, BF_REWRITE(True() ^ True()));

    BF_TEST(IsFalse(r));

    return 0;
}

int main() {
    BF_RUN_TEST(TestAndFold);
    BF_RUN_TEST(TestOrFold);
    BF_RUN_TEST(TestOrFold_BitwiseConstants);
    BF_RUN_TEST(TestXorFold);
    BF_RUN_TEST(TestXorFoldAllConstZero);
    return 0;
}