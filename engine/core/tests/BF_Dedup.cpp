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

static Expr* MakeOp(uint32_t id, OpType op, std::initializer_list<Expr*> in) {
    Expr* e = new Expr{};
    e->id = Ids::ExprId{id};
    e->op = op;
    e->inputs = in;
    return e;
}

int TestDedup() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);

    auto e1 = MakeOp(3, OpType::Xor, {x, y});
    auto e2 = MakeOp(4, OpType::Xor, {y, x});

    RuleEngine engine;
    Add_Bitwise_Simplify_Pipeline(engine);

    Expr* r1 = engine.ApplyUntilStable(e1);
    Expr* r2 = engine.ApplyUntilStable(e2);

    BF_TEST(r1 == r2);

    return 0;
}

int main() {
    BF_RUN_TEST(TestDedup);

    return 0;
}