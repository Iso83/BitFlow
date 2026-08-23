#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

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
        CPPTEST_ASSERT(r == x);
    }

    {
        BF_SAFE_REWRITE(r, BF_REWRITE(x & False() & True()));
        CPPTEST_ASSERT(IsFalse(r));
    }

    return 0;
}

int TestAndFold_BitwiseConstants() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_AndFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    {
        BF_SAFE_REWRITE(r, BF_REWRITE(C(0xFFu) & C(0x0Fu)));
        CPPTEST_ASSERT(EqualChunkValue(r, 0x0Fu));
    }

    {
        BF_SAFE_REWRITE(r, BF_REWRITE(C(0xFFu) & C(0x0Fu) & a));
        CPPTEST_ASSERT(Op(r) == OpType::And);
        CPPTEST_ASSERT(InputSize(r) == 2);
        CPPTEST_ASSERT(EqualChunkValue(Input(r, 0), 0x0Fu));
        CPPTEST_ASSERT(Input(r, 1) == a);
    }

    return 0;
}

int TestAndFold_AllConstants() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_AndFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    BF_SAFE_REWRITE(r, BF_REWRITE(C(528734635u) & C(2935074176u)));

    CPPTEST_ASSERT(EqualChunkValue(r, 243370368u));

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
        CPPTEST_ASSERT(r == x);
    }

    {
        BF_SAFE_REWRITE(r, BF_REWRITE(x | True() | False()));
        CPPTEST_ASSERT(IsTrue(r));
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
        CPPTEST_ASSERT(EqualChunkValue(r, 3u));
    }

    {
        BF_SAFE_REWRITE(r, BF_REWRITE(C(1) | C(2) | a));
        CPPTEST_ASSERT(Op(r) == OpType::Or);
        CPPTEST_ASSERT(InputSize(r) == 2);
        CPPTEST_ASSERT(EqualChunkValue(Input(r, 0), 3u));
        CPPTEST_ASSERT(Input(r, 1) == a);
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

    CPPTEST_ASSERT(r == x);
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

    CPPTEST_ASSERT(IsFalse(r));

    return 0;
}

int main() {
    CPPTEST_RUN(TestAndFold);
    CPPTEST_RUN(TestAndFold_BitwiseConstants);
    CPPTEST_RUN(TestAndFold_AllConstants);
    CPPTEST_RUN(TestOrFold);
    CPPTEST_RUN(TestOrFold_BitwiseConstants);
    CPPTEST_RUN(TestXorFold);
    CPPTEST_RUN(TestXorFoldAllConstZero);
    return 0;
}
