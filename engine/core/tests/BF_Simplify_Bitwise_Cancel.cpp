#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestAndCancelPair() {
    auto x = MakeVar(1);

    auto expr = MakeOp(2, OpType::And, {x, x});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Cancel_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->id == x->id);
    return 0;
}

int TestAndCancelMixed() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);

    auto expr = MakeOp(3, OpType::And, {x, y, x});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Cancel_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::And);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->id == y->id);
    return 0;
}

int TestOrCancelPair() {
    auto x = MakeVar(1);

    auto expr = MakeOp(2, OpType::Or, {x, x});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Or_Cancel_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->id == x->id);
    return 0;
}

int TestOrCancelMixed() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);

    auto expr = MakeOp(3, OpType::Or, {y, x, y});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Or_Cancel_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::Or);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id != result->inputs[1]->id);
    return 0;
}

int TestXorParityCancel_Pair() {
    auto a = MakeVar(1);

    auto expr = MakeOp(2, OpType::Xor, {a, a});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->isConst());
    BF_TEST(r->constValue == 0);
    return 0;
}

int TestXorParityCancel_ToSingle() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);

    auto expr = MakeOp(3, OpType::Xor, {x, x, y});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r == y);
    return 0;
}

int TestXorParityCancel_MixedToSingle() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);

    auto expr = MakeOp(3, OpType::Xor, {a, b, a});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r == b);
    return 0;
}

int TestXorParityCancel_MixedToXor() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);

    auto expr = MakeOp(10, OpType::Xor, {a, b, c, a});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::Xor);
    BF_TEST(r->inputs.size() == 2);
    BF_TEST(r->inputs[0] == b);
    BF_TEST(r->inputs[1] == c);
    return 0;
}

int TestXorParityCancel_AllEven() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);

    auto expr = MakeOp(3, OpType::Xor, {a, b, a, b});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->isConst());
    BF_TEST(r->constValue == 0);
    return 0;
}

int TestXorParityCancel_Triple() {
    auto a = MakeVar(1);

    auto expr = MakeOp(2, OpType::Xor, {a, a, a});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r == a);
    return 0;
}

int TestXorParityCancel_Frozen() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);

    auto expr = MakeOp(10, OpType::Xor, {a, b, a});
    expr->frozen = true;

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r != expr);
    BF_TEST(r == b);
    return 0;
}

int TestXorParity_WithConstCancel() {
    auto a = MakeVar(1);
    auto one = ConstPool::Get(1);

    auto expr = MakeOp(10, OpType::Xor, {a, one, a});

    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Bitwise_Rules(engine);

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->isConst());
    BF_TEST(r->constValue == 1);
    return 0;
}

int TestXorParity_WithConstMixed() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto one = ConstPool::Get(1);

    auto expr = MakeOp(10, OpType::Xor, {a, one, b, a});

    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Bitwise_Rules(engine);

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::Xor);
    BF_TEST(r->inputs.size() == 2);
    return 0;
}

int TestXorParity_RewriteKeepsCanonicalOrder() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);
    auto one = ConstPool::Get(1);

    auto expr = MakeOp(20, OpType::Xor, {c, a, b, one});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::Xor);
    BF_TEST(r->inputs.size() == 4);

    for (size_t i = 1; i < r->inputs.size(); ++i)
        BF_TEST(r->inputs[i - 1]->id.value() <= r->inputs[i]->id.value());

    return 0;
}

int TestXorParity_StructuralRotatePairCancelsToZero() {
    auto x = MakeVar(1);
    auto rotA = MakeOp(10, OpType::RotR, {x, MakeConst(11, 2)});
    auto rotB = MakeOp(12, OpType::RotR, {x, MakeConst(13, 2)});
    auto expr = MakeOp(14, OpType::Xor, {rotA, rotB});

    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Bitwise_Rules(engine);

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->isConst());
    BF_TEST(r->constValue == 0);
    return 0;
}

int TestXorParity_StructuralRotateDuplicateLeavesSingle() {
    auto x = MakeVar(1);
    auto rot2A = MakeOp(20, OpType::RotR, {x, MakeConst(21, 2)});
    auto rot13 = MakeOp(22, OpType::RotR, {x, MakeConst(23, 13)});
    auto rot2B = MakeOp(24, OpType::RotR, {x, MakeConst(25, 2)});
    auto expr = MakeOp(26, OpType::Xor, {rot2A, rot13, rot2B});

    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Bitwise_Rules(engine);

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r == rot13);
    return 0;
}

int main() {
    BF_RUN_TEST(TestAndCancelPair);
    BF_RUN_TEST(TestAndCancelMixed);
    BF_RUN_TEST(TestOrCancelPair);
    BF_RUN_TEST(TestOrCancelMixed);
    BF_RUN_TEST(TestXorParityCancel_Pair);
    BF_RUN_TEST(TestXorParityCancel_ToSingle);
    BF_RUN_TEST(TestXorParityCancel_MixedToSingle);
    BF_RUN_TEST(TestXorParityCancel_MixedToXor);
    BF_RUN_TEST(TestXorParityCancel_AllEven);
    BF_RUN_TEST(TestXorParityCancel_Triple);
    BF_RUN_TEST(TestXorParityCancel_Frozen);
    BF_RUN_TEST(TestXorParity_WithConstCancel);
    BF_RUN_TEST(TestXorParity_WithConstMixed);
    BF_RUN_TEST(TestXorParity_RewriteKeepsCanonicalOrder);
    BF_RUN_TEST(TestXorParity_StructuralRotatePairCancelsToZero);
    BF_RUN_TEST(TestXorParity_StructuralRotateDuplicateLeavesSingle);
    return 0;
}
