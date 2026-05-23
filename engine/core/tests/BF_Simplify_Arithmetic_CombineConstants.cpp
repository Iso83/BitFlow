#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestXorDedup() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_CombineConstants_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r1, Rewrite(engine, x ^ y));
    BF_SAFE_REWRITE(r2, Rewrite(engine, y ^ x));

    BF_TEST(Op(r1) == OpType::Xor);
    BF_TEST(Op(r2) == OpType::Xor);
    BF_TEST(Input(r1, 0) == Input(r2, 0));
    BF_TEST(Input(r1, 1) == Input(r2, 1));
    return 0;
}

int TestSimplify_ConstCombine_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_CombineConstants_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    {
        BF_SAFE_REWRITE(r, Rewrite(engine, C(5) + 7));
        BF_TEST(EqualChunkValue(r, 12u));
    }

    {
        BF_SAFE_REWRITE(r, Rewrite(engine, C(9) - 4));
        BF_TEST(EqualChunkValue(r, 5u));
    }
    {
        BF_SAFE_REWRITE(r, Rewrite(engine, C(5) - 6));
        BF_TEST(Op(r) == OpType::Neg);
        BF_TEST(EqualChunkValue(Input(r, 0), 1u));
    }
    {
        BF_SAFE_REWRITE(r, Rewrite(engine, C(3) * 6));
        BF_TEST(EqualChunkValue(r, 18u));
    }
    {
        BF_SAFE_REWRITE(r, Rewrite(engine, C(8) / 2));
        BF_TEST(EqualChunkValue(r, 4u));
    }
    {
        auto in = C(2) / 3;
        BF_SAFE_REWRITE(r, Rewrite(engine, in));
        BF_TEST(r == in);
    }
    return 0;
}

int main() {
    BF_RUN_TEST(TestXorDedup);
    BF_RUN_TEST(TestSimplify_ConstCombine_Basic);
    return 0;
}
