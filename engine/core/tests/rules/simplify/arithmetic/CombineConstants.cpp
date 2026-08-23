#include "TestAssert.h"

#include "common/Expr.h"
#include "common/Rule.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

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

    BF_SAFE_REWRITE(r1, BF_REWRITE(x ^ y));
    BF_SAFE_REWRITE(r2, BF_REWRITE(y ^ x));

    CPPTEST_ASSERT(Op(r1) == OpType::Xor);
    CPPTEST_ASSERT(Op(r2) == OpType::Xor);
    CPPTEST_ASSERT(Input(r1, 0) == Input(r2, 0));
    CPPTEST_ASSERT(Input(r1, 1) == Input(r2, 1));
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
        BF_SAFE_REWRITE(r, BF_REWRITE(C(5) + 7));
        CPPTEST_ASSERT(EqualChunkValue(r, 12u));
    }

    {
        BF_SAFE_REWRITE(r, BF_REWRITE(C(9) - 4));
        CPPTEST_ASSERT(EqualChunkValue(r, 5u));
    }
    {
        BF_SAFE_REWRITE(r, BF_REWRITE(C(5) - 6));
        CPPTEST_ASSERT(Op(r) == OpType::Neg);
        CPPTEST_ASSERT(EqualChunkValue(Input(r, 0), 1u));
    }
    {
        BF_SAFE_REWRITE(r, BF_REWRITE(C(3) * 6));
        CPPTEST_ASSERT(EqualChunkValue(r, 18u));
    }
    {
        BF_SAFE_REWRITE(r, BF_REWRITE(C(8) / 2));
        CPPTEST_ASSERT(EqualChunkValue(r, 4u));
    }
    {
        auto in = C(2) / 3;
        BF_SAFE_REWRITE(r, BF_REWRITE(in));
        CPPTEST_ASSERT(r == in);
    }
    return 0;
}

int main() {
    CPPTEST_RUN(TestXorDedup);
    CPPTEST_RUN(TestSimplify_ConstCombine_Basic);
    return 0;
}
