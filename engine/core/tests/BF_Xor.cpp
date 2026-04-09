#include <BitFlow/core/ConstPool.h>
#include <BitFlow/core/Expression.h>
#include <BitFlow/core/Rule.h>
#include <BitFlow/core/RuleEngine.h>
#include <BitFlow/core/RulePipeline.h>
#include <TestAssert.h>

using namespace BitFlow::Core;

static Expr* MakeVar(uint32_t id) {
    Expr* e = new Expr{};
    e->id = Ids::ExprId{id};
    e->op = OpType::Xor;
    return e;
}

static Expr* MakeConst(uint32_t id, uint32_t v) {
    Expr* e = new Expr{};
    e->id = Ids::ExprId{id};
    e->isConst = true;
    e->constValue = v;
    return e;
}

static Expr* MakeOp(uint32_t id, OpType op, std::initializer_list<Expr*> in) {
    Expr* e = new Expr{};
    e->id = Ids::ExprId{id};
    e->op = op;
    e->inputs = in;
    return e;
}

int TestZero() {
    auto x = MakeVar(10);
    auto zero = ConstPool::Get(0);

    auto xor1 = MakeOp(12, OpType::Xor, {x, zero});
    auto xor2 = MakeOp(13, OpType::Xor, {zero, xor1});

    RuleEngine engine;
    engine.AddRule(Get_Xor_Zero_Rule());

    Expr* result = engine.ApplyUntilStable(xor2);

    BF_TEST(result->id == x->id);
    return 0;
}

int TestCancel() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);

    auto expr = MakeOp(3, OpType::Xor, {x, x, y});

    RuleEngine engine;
    engine.AddRule(Get_Xor_Cancel_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    // verwacht: y
    BF_TEST(result->id == y->id);
    return 0;
}

int TestOrdering() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);

    // bewust unsorted
    auto expr = MakeOp(3, OpType::Xor, {y, x, y, x});

    RuleEngine engine;
    engine.AddRule(Get_Order_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    // verwacht: [x, x, y, y]
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->id == x->id);
    BF_TEST(result->inputs[2]->id == y->id);
    BF_TEST(result->inputs[3]->id == y->id);

    return 0;
}

int TestFlatten() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);
    auto z = MakeVar(3);

    auto inner = MakeOp(4, OpType::Xor, {x, y});
    auto outer = MakeOp(5, OpType::Xor, {inner, z});

    RuleEngine engine;
    engine.AddRule(Get_Flatten_Rule());

    Expr* result = engine.ApplyUntilStable(outer);

    BF_TEST(result->inputs.size() == 3);
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->id == y->id);
    BF_TEST(result->inputs[2]->id == z->id);

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

int main() {
    BF_RUN_TEST(TestZero);
    BF_RUN_TEST(TestCancel);
    BF_RUN_TEST(TestOrdering);
    BF_RUN_TEST(TestFlatten);
    BF_RUN_TEST(TestXorFold);

    return 0;
}