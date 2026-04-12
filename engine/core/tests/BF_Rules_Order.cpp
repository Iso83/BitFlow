#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestXorOrdering() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);

    // bewust unsorted
    auto expr = MakeOp(3, OpType::Xor, {y, x, y, x});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Order_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    // verwacht: [x, x, y, y]
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->id == x->id);
    BF_TEST(result->inputs[2]->id == y->id);
    BF_TEST(result->inputs[3]->id == y->id);
    return 0;
}

int main() {
    BF_RUN_TEST(TestXorOrdering);
    return 0;
}