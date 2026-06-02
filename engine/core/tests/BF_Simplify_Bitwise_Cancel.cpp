#include <ExprTestUtils.h>
#include <RuleTestUtils.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestXorParityCancel_Pair() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorZero_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(a ^ a));

    BF_TEST(EqualChunkValue(r, 0u));
    return 0;
}

int TestXorParityCancel_ToSingle() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorZero_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r, BF_REWRITE(x ^ x ^ y));

    BF_TEST(r == y);
    return 0;
}

int TestXorParityCancel_MixedToXor() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorZero_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE(a ^ b ^ c ^ a));

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(Input(r, 0) == b);
    BF_TEST(Input(r, 1) == c);
    return 0;
}

int TestXorParityCancel_AllEven() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorZero_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE(a ^ b ^ a ^ b));

    BF_TEST(EqualChunkValue(r, 0u));
    return 0;
}

int TestXorParityCancel_Triple() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorZero_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(a ^ a ^ a));

    BF_TEST(r == a);
    return 0;
}

int TestXorParity_RewriteKeepsCanonicalOrder() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorZero_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE(c ^ a ^ b ^ a));

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(Input(r, 0) == b);
    BF_TEST(Input(r, 1) == c);
    return 0;
}

int main() {
    BF_RUN_TEST(TestXorParityCancel_Pair);
    BF_RUN_TEST(TestXorParityCancel_ToSingle);
    BF_RUN_TEST(TestXorParityCancel_MixedToXor);
    BF_RUN_TEST(TestXorParityCancel_AllEven);
    BF_RUN_TEST(TestXorParityCancel_Triple);
    BF_RUN_TEST(TestXorParity_RewriteKeepsCanonicalOrder);
    return 0;
}
