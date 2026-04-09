#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestXorFlatten() {
    auto x = MakeVar(1, OpType ::Xor);
    auto y = MakeVar(2, OpType ::Xor);
    auto z = MakeVar(3, OpType ::Xor);

    auto inner = MakeOp(4, OpType::Xor, {x, y});
    auto outer = MakeOp(5, OpType::Xor, {inner, z});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());

    Expr* result = engine.ApplyUntilStable(outer);

    BF_TEST(result->inputs.size() == 3);
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->id == y->id);
    BF_TEST(result->inputs[2]->id == z->id);
    return 0;
}

int main() {
    BF_RUN_TEST(TestXorFlatten);
    return 0;
}