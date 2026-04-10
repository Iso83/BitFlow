#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestAndDistribute() {
    auto a = MakeVar(1, OpType::And);
    auto b = MakeVar(2, OpType::And);
    auto c = MakeVar(3, OpType::And);

    auto orNode = MakeOp(10, OpType::Or, {b, c});
    auto expr = MakeOp(11, OpType::And, {a, orNode});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Factorize::Get_And_Distribute_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::Or);
    BF_TEST(r->inputs.size() == 2);

    BF_TEST(r->inputs[0]->op == OpType::And);
    BF_TEST(r->inputs[1]->op == OpType::And);

    return 0;
}

int main() {
    BF_RUN_TEST(TestAndDistribute);
    return 0;
}