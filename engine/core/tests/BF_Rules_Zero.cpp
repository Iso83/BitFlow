#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

#pragma region Add-Zero
int TestAddZero_EliminatesNeutralTerms() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);
    auto zero = ConstPool::Get(0);

    auto add = MakeOp(3, OpType::Add, {zero, x, zero, y, zero});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_Add_Zero_Rule());

    Expr* result = engine.ApplyUntilStable(add);

    BF_TEST(result->op == OpType::Add);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->id == y->id);
    return 0;
}

int TestAddZero_AllZerosCollapsesToZero() {
    auto zero = ConstPool::Get(0);
    auto add = MakeOp(10, OpType::Add, {zero, zero, zero});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_Add_Zero_Rule());

    Expr* result = engine.ApplyUntilStable(add);

    BF_TEST(result->isConst());
    BF_TEST(result->constValue == 0);
    return 0;
}

int TestAddZero_Regression_OrderingPreserved() {
    auto a = MakeVar(40);
    auto b = MakeVar(20);
    auto c = MakeVar(30);
    auto zero = ConstPool::Get(0);

    auto add = MakeOp(50, OpType::Add, {b, zero, c, zero, a});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Get_Add_Zero_Rule());

    Expr* result = engine.ApplyUntilStable(add);

    BF_TEST(result->op == OpType::Add);
    BF_TEST(result->inputs.size() == 3);
    BF_TEST(result->inputs[0]->id == b->id);
    BF_TEST(result->inputs[1]->id == c->id);
    BF_TEST(result->inputs[2]->id == a->id);
    return 0;
}
#pragma endregion

#pragma region Xor-Zero
int TestXorZero() {
    auto x = MakeVar(10);
    auto zero = ConstPool::Get(0);

    auto xor1 = MakeOp(12, OpType::Xor, {x, zero});
    auto xor2 = MakeOp(13, OpType::Xor, {zero, xor1});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_Xor_Zero_Rule());

    Expr* result = engine.ApplyUntilStable(xor2);

    BF_TEST(result->id == x->id);
    return 0;
}
#pragma endregion

int main() {
    BF_RUN_TEST(TestAddZero_EliminatesNeutralTerms);
    BF_RUN_TEST(TestAddZero_AllZerosCollapsesToZero);
    BF_RUN_TEST(TestAddZero_Regression_OrderingPreserved);
    BF_RUN_TEST(TestXorZero);
    return 0;
}
