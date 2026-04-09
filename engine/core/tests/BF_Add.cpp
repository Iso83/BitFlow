#include <BitFlow/core/Expression.h>
#include <BitFlow/core/Rule.h>
#include <BitFlow/core/RuleEngine.h>
#include <BitFlow/core/ConstPool.h>
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
    auto x = MakeVar(1);
    auto zero = ConstPool::Get(0);

    auto add1 = MakeOp(3, OpType::Add, {x, zero});
    auto add2 = MakeOp(4, OpType::Add, {add1, zero});

    RuleEngine engine;
    engine.AddRule(Get_Add_Zero_Rule());

    Expr* result = engine.ApplyUntilStable(add2);

    BF_TEST(result->id == x->id);
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

int main() {
    BF_RUN_TEST(TestZero);
    BF_RUN_TEST(TestAndFold);

    return 0;
}