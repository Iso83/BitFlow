#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

#include <BitFlow/engine/core/rules/RulePipeline.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestXorParity_WithConstCancel() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyBitwise());

    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(a ^ 1 ^ a));

    CPPTEST_ASSERT(EqualChunkValue(r, 1u));
    return 0;
}

int TestXorParity_WithConstMixed() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyBitwise());

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE(a ^ 1 ^ b ^ a));

    CPPTEST_ASSERT(Op(r) == OpType::Xor);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == b; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 1u); }));
    return 0;
}

int TestXorParity_StructuralRotatePairCancelsToZero() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyBitwise());

    auto x = V("x");
    auto term = x.RotR(2);

    BF_SAFE_REWRITE(r, BF_REWRITE(term ^ term));

    CPPTEST_ASSERT(EqualChunkValue(r, 0u));
    return 0;
}

int TestXorParity_StructuralRotateDuplicateLeavesSingle() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyBitwise());

    auto x = V("x");
    auto term = x.RotR(2);

    BF_SAFE_REWRITE(r, BF_REWRITE(term ^ x.RotR(13) ^ term));

    CPPTEST_ASSERT(Op(r) == OpType::RotR);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(Input(r, 0) == x);
    CPPTEST_ASSERT(EqualChunkValue(Input(r, 1), 13u));

    return 0;
}

int main() {
    CPPTEST_RUN(TestXorParity_WithConstCancel);
    CPPTEST_RUN(TestXorParity_WithConstMixed);
    CPPTEST_RUN(TestXorParity_StructuralRotatePairCancelsToZero);
    CPPTEST_RUN(TestXorParity_StructuralRotateDuplicateLeavesSingle);
    return 0;
}
