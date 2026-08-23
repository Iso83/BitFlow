#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

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

    CPPTEST_ASSERT(EqualChunkValue(r, 0u));
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

    CPPTEST_ASSERT(r == y);
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

    CPPTEST_ASSERT(Op(r) == OpType::Xor);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(Input(r, 0) == b);
    CPPTEST_ASSERT(Input(r, 1) == c);
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

    CPPTEST_ASSERT(EqualChunkValue(r, 0u));
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

    CPPTEST_ASSERT(r == a);
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

    CPPTEST_ASSERT(Op(r) == OpType::Xor);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(Input(r, 0) == b);
    CPPTEST_ASSERT(Input(r, 1) == c);
    return 0;
}

int main() {
    CPPTEST_RUN(TestXorParityCancel_Pair);
    CPPTEST_RUN(TestXorParityCancel_ToSingle);
    CPPTEST_RUN(TestXorParityCancel_MixedToXor);
    CPPTEST_RUN(TestXorParityCancel_AllEven);
    CPPTEST_RUN(TestXorParityCancel_Triple);
    CPPTEST_RUN(TestXorParity_RewriteKeepsCanonicalOrder);
    return 0;
}
