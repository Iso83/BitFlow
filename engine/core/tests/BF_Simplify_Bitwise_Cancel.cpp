#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestAndCancelPair() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Cancel_Rule());
    auto x = V("x");

    BF_TEST(Rewrite(engine, x & x) == x);
    return 0;
}

int TestAndCancelMixed() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Cancel_Rule());
    auto x = V("x");
    auto y = V("y");
    auto r = Rewrite(engine, x & y & x);
    auto out = ExprOf(r);

    BF_TEST(out.op == OpType::And);
    BF_TEST(out.inputs.size() == 2);
    BF_TEST(ERef(out.inputs[0]) == x);
    BF_TEST(ERef(out.inputs[1]) == y);

    return 0;
}

int TestOrCancelPair() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Or_Cancel_Rule());
    auto x = V("x");

    BF_TEST(Rewrite(engine, x | x) == x);
    return 0;
}

int TestOrCancelMixed() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Or_Cancel_Rule());
    auto x = V("x");
    auto y = V("y");
    auto r = Rewrite(engine, y | x | y);
    auto out = ExprOf(r);

    BF_TEST(out.op == OpType::Or);
    BF_TEST(out.inputs.size() == 2);
    BF_TEST(ERef(out.inputs[0]) == x);
    BF_TEST(ERef(out.inputs[1]) == y);
    return 0;
}

int TestXorParityCancel_Pair() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());
    auto a = V("a");
    auto r = Rewrite(engine, a ^ a);

    BF_TEST(EqualChunkValue(r, 0u));
    return 0;
}

int TestXorParityCancel_ToSingle() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());
    auto x = V("x");
    auto y = V("y");

    BF_TEST(Rewrite(engine, x ^ x ^ y) == y);
    return 0;
}

int TestXorParityCancel_MixedToXor() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());
    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto r = Rewrite(engine, a ^ b ^ c ^ a);
    auto out = ExprOf(r);

    BF_TEST(out.op == OpType::Xor);
    BF_TEST(out.inputs.size() == 2);
    BF_TEST(ERef(out.inputs[0]) == b);
    BF_TEST(ERef(out.inputs[1]) == c);
    return 0;
}

int TestXorParityCancel_AllEven() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());
    auto a = V("a");
    auto b = V("b");
    auto r = Rewrite(engine, a ^ b ^ a ^ b);

    BF_TEST(EqualChunkValue(r, 0u));
    return 0;
}

int TestXorParityCancel_Triple() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());
    auto a = V("a");

    BF_TEST(Rewrite(engine, a ^ a ^ a) == a);
    return 0;
}

int TestXorParity_WithConstCancel() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyBitwise());
    auto a = V("a");
    auto r = Rewrite(engine, a ^ 1 ^ a);

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
    auto r = Rewrite(engine, a ^ 1 ^ b ^ a);
    auto out = ExprOf(r);

    BF_TEST(out.op == OpType::Xor);
    BF_TEST(out.inputs.size() == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == b; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 1u); }));
    return 0;
}

int TestXorParity_RewriteKeepsCanonicalOrder() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());
    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto r = Rewrite(engine, c ^ a ^ b ^ a);
    auto out = ExprOf(r);

    BF_TEST(out.op == OpType::Xor);
    BF_TEST(out.inputs.size() == 2);
    BF_TEST(ERef(out.inputs[0]) == b);
    BF_TEST(ERef(out.inputs[1]) == c);
    return 0;
}

int TestXorParity_StructuralRotatePairCancelsToZero() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyBitwise());
    auto x = V("x");
    auto r = Rewrite(engine, x.RotR(2) ^ x.RotR(x));

    BF_TEST(EqualChunkValue(r, 0u));
    return 0;
}

int TestXorParity_StructuralRotateDuplicateLeavesSingle() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyBitwise());
    auto x = V("x");
    auto r = Rewrite(engine, x.RotR(2) ^ x.RotR(13) ^ x.RotR(2));

    auto out = ExprOf(r);

    BF_TEST(out.op == OpType::RotR);
    BF_TEST(out.inputs.size() == 2);
    BF_TEST(ERef(out.inputs[0]) == x);
    BF_TEST(EqualChunkValue(ERef(out.inputs[1]), 13u));

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
    BF_RUN_TEST(TestXorParity_WithConstCancel);
    BF_RUN_TEST(TestXorParity_WithConstMixed);
    BF_RUN_TEST(TestXorParity_RewriteKeepsCanonicalOrder);
    BF_RUN_TEST(TestXorParity_StructuralRotatePairCancelsToZero);
    BF_RUN_TEST(TestXorParity_StructuralRotateDuplicateLeavesSingle);
    return 0;
}
