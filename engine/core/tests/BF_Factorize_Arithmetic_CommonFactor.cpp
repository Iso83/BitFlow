#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

static RuleEngine MakeArithmeticCanonicalEngine() {
    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyArithmetic());
    engine.Merge(BuildFactorizeArithmetic());
    return engine;
}

int TestCanonical_MulCoeffOrder_2a_plus_a() {
    MakeExprStore(32);

    RuleEngine engine = MakeArithmeticCanonicalEngine();
    auto a = V("a");

    {
        auto out = Rewrite(engine, (C(2) * a) + a);
        BF_TEST(Op(out) == OpType::Mul);
        BF_TEST(InputSize(out) == 2);
        BF_TEST(Input(out, 0) == a);
        BF_TEST(EqualChunkValue(Input(out, 1), 3u));
    }

    {
        auto out = Rewrite(engine, (a * 2) + a);
        BF_TEST(Op(out) == OpType::Mul);
        BF_TEST(InputSize(out) == 2);
        BF_TEST(Input(out, 0) == a);
        BF_TEST(EqualChunkValue(Input(out, 1), 3u));
    }
    return 0;
}

int TestCanonical_a_b_plus_b_a() {
    MakeExprStore(32);

    RuleEngine engine = MakeArithmeticCanonicalEngine();
    auto a = V("a");
    auto b = V("b");
    auto out = Rewrite(engine, (a * b) + (b * a));

    BF_TEST(Op(out) == OpType::Mul);
    BF_TEST(InputSize(out) == 3);
    BF_TEST(Input(out, 0) == a);
    BF_TEST(Input(out, 1) == b);
    BF_TEST(EqualChunkValue(Input(out, 2), 2u));
    return 0;
}

int TestCanonical_combineMulConstants_Order() {
    MakeExprStore(32);

    RuleEngine engine = MakeArithmeticCanonicalEngine();
    auto a = V("a");
    auto out = Rewrite(engine, C(2) * a * 3);

    BF_TEST(Op(out) == OpType::Mul);
    BF_TEST(InputSize(out) == 2);
    BF_TEST(Input(out, 0) == a);
    BF_TEST(EqualChunkValue(Input(out, 1), 6u));
    return 0;
}

int main() {
    BF_RUN_TEST(TestCanonical_MulCoeffOrder_2a_plus_a);
    BF_RUN_TEST(TestCanonical_a_b_plus_b_a);
    BF_RUN_TEST(TestCanonical_combineMulConstants_Order);
    return 0;
}
