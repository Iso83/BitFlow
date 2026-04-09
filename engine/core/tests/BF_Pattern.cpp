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

int TestXorAndCommonFactor() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);

    auto and1 = MakeOp(10, OpType::And, {a, b});
    auto and2 = MakeOp(11, OpType::And, {a, c});
    auto expr = MakeOp(12, OpType::Xor, {and1, and2});

    RuleEngine engine;
    Add_Bitwise_Simplify_Pipeline(engine);

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::And);
    BF_TEST(result->inputs.size() == 2);

    Expr* left = result->inputs[0];
    Expr* right = result->inputs[1];

    Expr* common = nullptr;
    Expr* inner = nullptr;

    if (left->id == a->id) {
        common = left;
        inner = right;
    } else if (right->id == a->id) {
        common = right;
        inner = left;
    } else
        BF_TEST(false);

    BF_TEST(inner->op == OpType::Xor);
    BF_TEST(inner->inputs.size() == 2);

    auto x0 = inner->inputs[0];
    auto x1 = inner->inputs[1];

    BF_TEST((x0->id == b->id && x1->id == c->id) || (x0->id == c->id && x1->id == b->id));

    return 0;
}

int TestXorXorCancelPair() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);

    auto x1 = MakeOp(10, OpType::Xor, {a, b});
    auto x2 = MakeOp(11, OpType::Xor, {a, c});
    auto expr = MakeOp(12, OpType::Xor, {x1, x2});

    RuleEngine engine;
    Add_Bitwise_Simplify_Pipeline(engine);

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::Xor);
    BF_TEST(result->inputs.size() == 2);

    auto i0 = result->inputs[0];
    auto i1 = result->inputs[1];

    BF_TEST((i0->id == b->id && i1->id == c->id) || (i0->id == c->id && i1->id == b->id));

    return 0;
}

int main() {
    BF_RUN_TEST(TestXorAndCommonFactor);
    BF_RUN_TEST(TestXorXorCancelPair);

    return 0;
}