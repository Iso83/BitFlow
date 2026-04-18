#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestCH_ExpandsToBooleanForm() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);
    auto z = MakeVar(3);

    auto expr = MakeOp(10, OpType::Ch, {x, y, z});

    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_SHA_Rules(engine);

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::Xor);
    BF_TEST(r->inputs.size() == 2);
    BF_TEST(r->inputs[0]->op == OpType::And);
    BF_TEST(r->inputs[1]->op == OpType::And);
    const bool rhsHasNot = r->inputs[1]->inputs[0]->op == OpType::Not || r->inputs[1]->inputs[1]->op == OpType::Not;
    BF_TEST(rhsHasNot);
    return 0;
}

int main() {
    BF_RUN_TEST(TestCH_ExpandsToBooleanForm);
    return 0;
}
