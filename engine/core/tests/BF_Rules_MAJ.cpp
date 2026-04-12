#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestMAJ_Basic() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);
    auto z = MakeVar(3);

    auto xy = MakeOp(10, OpType::And, {x, y});
    auto xz = MakeOp(11, OpType::And, {x, z});
    auto yz = MakeOp(12, OpType::And, {y, z});

    auto expr = MakeOp(13, OpType::Xor, {xy, xz, yz});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Get_MAJ_Simplify_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::Xor);
    BF_TEST(r->inputs.size() == 3);
    return 0;
}

int TestMAJ_Collapse() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);

    auto xx = MakeOp(10, OpType::And, {x, x});
    auto xy = MakeOp(11, OpType::And, {x, y});
    auto xy2 = MakeOp(12, OpType::And, {y, x});

    auto expr = MakeOp(13, OpType::Xor, {xx, xy, xy2});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Get_MAJ_Simplify_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r == x);
    return 0;
}

int main() {
    BF_RUN_TEST(TestMAJ_Basic);
    BF_RUN_TEST(TestMAJ_Collapse);
    return 0;
}