#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestAddFold() {
    auto x = MakeVar(1);
    auto c1 = MakeConst(2, 10);
    auto c2 = MakeConst(3, 20);

    auto expr = MakeOp(4, OpType::Add, {x, c1, c2});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_Add_Fold_Rule());
    engine.AddRule(Simplify::Get_Add_Zero_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::Add);
    BF_TEST(result->inputs.size() == 2);

    Expr* a = result->inputs[0];
    Expr* b = result->inputs[1];

    if (a->isConst()) {
        BF_TEST(a->constValue == 30);
        BF_TEST(b->id == x->id);
    } else {
        BF_TEST(a->id == x->id);
        BF_TEST(b->isConst());
        BF_TEST(b->constValue == 30);
    }

    return 0;
}

int TestAndFold() {
    auto x = MakeVar(1);
    auto zero = ConstPool::Get(0);
    auto one = ConstPool::Get(1);

    auto expr = MakeOp(2, OpType::And, {x, one, one});
    auto expr2 = MakeOp(3, OpType::And, {x, zero, one});

    RuleEngine engine;
    Add_Bitwise_Simplify_Pipeline(engine);

    Expr* r1 = engine.ApplyUntilStable(expr);
    BF_TEST(r1->id == x->id);

    Expr* r2 = engine.ApplyUntilStable(expr2);
    BF_TEST(r2->id == zero->id);

    return 0;
}

int TestOrFold() {
    auto x = MakeVar(1);
    auto zero = ConstPool::Get(0);
    auto one = ConstPool::Get(1);

    auto expr = MakeOp(2, OpType::Or, {x, zero, zero});
    auto expr2 = MakeOp(3, OpType::Or, {x, one, zero});

    RuleEngine engine;
    Add_Bitwise_Simplify_Pipeline(engine);

    Expr* r1 = engine.ApplyUntilStable(expr);
    BF_TEST(r1->id == x->id);

    Expr* r2 = engine.ApplyUntilStable(expr2);
    BF_TEST(r2->id == one->id);

    return 0;
}

int TestXorFold() {
    auto x = MakeVar(1);
    auto c1 = MakeConst(2, 1);
    auto c2 = MakeConst(3, 1);

    auto expr = MakeOp(4, OpType::Xor, {x, c1, c2});

    RuleEngine engine;
    Add_Bitwise_Simplify_Pipeline(engine);

    Expr* result = engine.ApplyUntilStable(expr);

    // 1 ^ 1 = 0 → x ^ 0 → x
    BF_TEST(result->id == x->id);

    return 0;
}

int TestXorFoldAllConstZero() {
    auto c1 = MakeConst(2, 1);
    auto c2 = MakeConst(3, 1);

    auto expr = MakeOp(4, OpType::Xor, {c1, c2});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_Xor_Fold_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->isConst());
    BF_TEST(result->constValue == 0);
    return 0;
}

int main() {
    BF_RUN_TEST(TestAddFold);
    BF_RUN_TEST(TestAndFold);
    BF_RUN_TEST(TestOrFold);
    BF_RUN_TEST(TestXorFold);
    BF_RUN_TEST(TestXorFoldAllConstZero);
    return 0;
}