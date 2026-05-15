#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestXorParity_WithConstCancel() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyBitwise());

    auto a = V("a");

    BF_SAFE_REWRITE(r, Rewrite(engine, a ^ 1 ^ a));

    BF_TEST(EqualChunkValue(r, 1u));
    return 0;
}

int TestXorParity_WithConstMixed() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyBitwise());

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, Rewrite(engine, a ^ 1 ^ b ^ a));

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == b; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 1u); }));
    return 0;
}

int TestXorParity_StructuralRotatePairCancelsToZero() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyBitwise());

    auto x = V("x");
    auto term = x.RotR(2);

    BF_SAFE_REWRITE(r, Rewrite(engine, term ^ term));

    BF_TEST(EqualChunkValue(r, 0u));
    return 0;
}

int TestXorParity_StructuralRotateDuplicateLeavesSingle() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyBitwise());

    auto x = V("x");
    auto term = x.RotR(2);

    BF_SAFE_REWRITE(r, Rewrite(engine, term ^ x.RotR(13) ^ term));

    BF_TEST(Op(r) == OpType::RotR);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(Input(r, 0) == x);
    BF_TEST(EqualChunkValue(Input(r, 1), 13u));

    return 0;
}

int main() {
    BF_RUN_TEST(TestXorParity_WithConstCancel);
    BF_RUN_TEST(TestXorParity_WithConstMixed);
    BF_RUN_TEST(TestXorParity_StructuralRotatePairCancelsToZero);
    BF_RUN_TEST(TestXorParity_StructuralRotateDuplicateLeavesSingle);
    return 0;
}
