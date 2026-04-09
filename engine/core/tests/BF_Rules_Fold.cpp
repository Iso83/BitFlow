#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestAndFold() {
    auto x = MakeVar(1, OpType ::And);
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
    auto x = MakeVar(1, OpType::Or);
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
    auto x = MakeVar(1, OpType::Xor);
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
    BF_RUN_TEST(TestAndFold);
    BF_RUN_TEST(TestOrFold);
    BF_RUN_TEST(TestXorFold);
    return 0;
}