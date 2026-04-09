#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestXorZero() {
    auto x = MakeVar(10, OpType::Xor);
    auto zero = ConstPool::Get(0);

    auto xor1 = MakeOp(12, OpType::Xor, {x, zero});
    auto xor2 = MakeOp(13, OpType::Xor, {zero, xor1});

    RuleEngine engine;
    engine.AddRule(Simplify::Get_Simplify_Xor_Zero_Rule());

    Expr* result = engine.ApplyUntilStable(xor2);

    BF_TEST(result->id == x->id);
    return 0;
}

int TestXorCancel() {
    auto x = MakeVar(1, OpType::Xor);
    auto y = MakeVar(2, OpType::Xor);

    auto expr = MakeOp(3, OpType::Xor, {x, x, y});

    RuleEngine engine;
    engine.AddRule(Simplify::Get_Simplify_Xor_Cancel_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    // verwacht: y
    BF_TEST(result->id == y->id);
    return 0;
}

int TestXorOrdering() {
    auto x = MakeVar(1, OpType::Xor);
    auto y = MakeVar(2, OpType::Xor);

    // bewust unsorted
    auto expr = MakeOp(3, OpType::Xor, {y, x, y, x});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Normalize_Order_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    // verwacht: [x, x, y, y]
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->id == x->id);
    BF_TEST(result->inputs[2]->id == y->id);
    BF_TEST(result->inputs[3]->id == y->id);
    return 0;
}

int TestXorFlatten() {
    auto x = MakeVar(1, OpType ::Xor);
    auto y = MakeVar(2, OpType ::Xor);
    auto z = MakeVar(3, OpType ::Xor);

    auto inner = MakeOp(4, OpType::Xor, {x, y});
    auto outer = MakeOp(5, OpType::Xor, {inner, z});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Normalize_Flatten_Rule());

    Expr* result = engine.ApplyUntilStable(outer);

    BF_TEST(result->inputs.size() == 3);
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->id == y->id);
    BF_TEST(result->inputs[2]->id == z->id);
    return 0;
}

int TestXorFold() {
    auto x = MakeVar(1, OpType ::Xor);
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

int main() {
    BF_RUN_TEST(TestXorZero);
    BF_RUN_TEST(TestXorCancel);
    BF_RUN_TEST(TestXorOrdering);
    BF_RUN_TEST(TestXorFlatten);
    BF_RUN_TEST(TestXorFold);
    return 0;
}