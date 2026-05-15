#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestAndCancelPair() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_AndCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x & x));

    BF_TEST(r == x);
    return 0;
}

int TestAndCancelMixed() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_AndCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r, Rewrite(engine, x & y & x));

    BF_TEST(Op(r) == OpType::And);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(Input(r, 0) == x);
    BF_TEST(Input(r, 1) == y);

    return 0;
}

int TestOrCancelPair() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_OrCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x | x));

    BF_TEST(r == x);
    return 0;
}

int TestOrCancelMixed() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_OrCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r, Rewrite(engine, y | x | y));

    BF_TEST(Op(r) == OpType::Or);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(Input(r, 0) == x);
    BF_TEST(Input(r, 1) == y);
    return 0;
}

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

    BF_SAFE_REWRITE(r, Rewrite(engine, a ^ a));

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

    BF_SAFE_REWRITE(r, Rewrite(engine, x ^ x ^ y));

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

    BF_SAFE_REWRITE(r, Rewrite(engine, a ^ b ^ c ^ a));

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

    BF_SAFE_REWRITE(r, Rewrite(engine, a ^ b ^ a ^ b));

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

    BF_SAFE_REWRITE(r, Rewrite(engine, a ^ a ^ a));

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

    BF_SAFE_REWRITE(r, Rewrite(engine, c ^ a ^ b ^ a));

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(Input(r, 0) == b);
    BF_TEST(Input(r, 1) == c);
    return 0;
}

int main() {
    BF_RUN_TEST(TestAndCancelPair);
    BF_RUN_TEST(TestAndCancelMixed);
    BF_RUN_TEST(TestOrCancelPair);
    BF_RUN_TEST(TestOrCancelMixed);
    BF_RUN_TEST(TestXorParityCancel_Pair);
    BF_RUN_TEST(TestXorParityCancel_ToSingle);
    BF_RUN_TEST(TestXorParityCancel_MixedToXor);
    BF_RUN_TEST(TestXorParityCancel_AllEven);
    BF_RUN_TEST(TestXorParityCancel_Triple);
    BF_RUN_TEST(TestXorParity_RewriteKeepsCanonicalOrder);
    return 0;
}
