#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestAndXorReduction_RightXor() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);
    auto xorNode = MakeOp(3, OpType::Xor, {x, y});
    auto expr = MakeOp(4, OpType::And, {x, xorNode});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::And);
    BF_TEST(r->inputs.size() == 2);
    BF_TEST(r->inputs[0] == x);
    BF_TEST(r->inputs[1]->op == OpType::Not);
    BF_TEST(r->inputs[1]->inputs.size() == 1);
    BF_TEST(r->inputs[1]->inputs[0] == y);
    return 0;
}

int TestAndXorReduction_LeftXor() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);
    auto xorNode = MakeOp(3, OpType::Xor, {x, y});
    auto expr = MakeOp(4, OpType::And, {xorNode, x});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::And);
    BF_TEST(r->inputs.size() == 2);
    BF_TEST(r->inputs[0] == x);
    BF_TEST(r->inputs[1]->op == OpType::Not);
    BF_TEST(r->inputs[1]->inputs.size() == 1);
    BF_TEST(r->inputs[1]->inputs[0] == y);
    return 0;
}

int main() {
    BF_RUN_TEST(TestAndXorReduction_RightXor);
    BF_RUN_TEST(TestAndXorReduction_LeftXor);
    return 0;
}
