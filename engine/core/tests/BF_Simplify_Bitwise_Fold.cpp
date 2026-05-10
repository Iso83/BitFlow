#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

static RuleEngine MakeEngine() {

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyBitwise());
    return engine;
}

int TestAndFold() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();
    auto x = V("x");

    {
        auto r = Rewrite(engine, x & True() & True());
        BF_TEST(r == x);
    }

    {
        auto r = Rewrite(engine, x & False() & True());
        BF_TEST(IsFalse(r));
    }

    return 0;
}

int TestOrFold() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();
    auto x = V("x");

    {
        auto r = Rewrite(engine, x | False() | False());

        BF_TEST(r == x);
    }

    {
        auto r = Rewrite(engine, x | True() | False());

        BF_TEST(IsTrue(r));
    }

    return 0;
}

int TestXorFold() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();
    auto x = V("x");
    auto r = Rewrite(engine, x ^ True() ^ True());

    // true ^ true = false
    // x ^ false = x

    BF_TEST(r == x);

    return 0;
}

int TestXorFoldAllConstZero() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Fold_Rule());

    auto r = Rewrite(engine, True() ^ True());

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