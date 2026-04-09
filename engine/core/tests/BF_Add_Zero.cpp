#include <BitFlow/core/Expression.h>
#include <BitFlow/core/Rule.h>
#include <BitFlow/core/RuleEngine.h>
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

int main() {
    auto x = MakeVar(1);
    auto zero = MakeConst(2, 0);

    auto add1 = MakeOp(3, OpType::Add, {x, zero});
    auto add2 = MakeOp(4, OpType::Add, {add1, zero});

    RuleEngine engine;
    engine.AddRule(Get_Add_Zero_Rule());

    Expr* result = engine.ApplyUntilStable(add2);

    BF_TEST(result->id == x->id);
    return 0;
}