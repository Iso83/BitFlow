#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestMAJ_ExpandsToBooleanForm() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);
    auto z = MakeVar(3);

    auto expr = MakeOp(20, OpType::Maj, {x, y, z});

    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_SHA_Rules(engine);

    Expr* r = engine.Rewrite(expr);

    BF_TEST(r->op == OpType::Xor);
    BF_TEST(r->inputs.size() == 3);
    BF_TEST(r->inputs[0]->op == OpType::And);
    BF_TEST(r->inputs[1]->op == OpType::And);
    BF_TEST(r->inputs[2]->op == OpType::And);
    return 0;
}

int main() {
    BF_RUN_TEST(TestMAJ_ExpandsToBooleanForm);
    return 0;
}
