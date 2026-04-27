#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestXorFlatten() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);
    auto z = MakeVar(3);

    auto inner = MakeOp(4, OpType::Xor, {x, y});
    auto outer = MakeOp(5, OpType::Xor, {inner, z});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());

    Expr* result = engine.Rewrite(outer);

    BF_TEST(result != outer);
    BF_TEST(result->inputs.size() == 3);
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->id == y->id);
    BF_TEST(result->inputs[2]->id == z->id);
    return 0;
}

int TestNotNotDoesNotFlatten() {
    auto x = MakeVar(1);

    auto inner = MakeOp(10, OpType::Not, {x});
    auto outer = MakeOp(11, OpType::Not, {inner});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());

    Expr* result = engine.Rewrite(outer);

    BF_TEST(result->op == OpType::Not);
    BF_TEST(result->inputs.size() == 1);
    BF_TEST(result->inputs[0]->op == OpType::Not);
    BF_TEST(result->inputs[0]->inputs.size() == 1);
    BF_TEST(result->inputs[0]->inputs[0]->id == x->id);

    return 0;
}

int main() {
    BF_RUN_TEST(TestXorFlatten);
    BF_RUN_TEST(TestNotNotDoesNotFlatten);
    return 0;
}
