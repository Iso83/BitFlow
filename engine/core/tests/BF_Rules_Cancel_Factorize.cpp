#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestXorXorCancelPair() {
    auto a = MakeVar(40, OpType::Xor);
    auto b = MakeVar(41, OpType::Xor);
    auto c = MakeVar(42, OpType::Xor);

    auto x1 = MakeOp(50, OpType::Xor, {a, b});
    auto x2 = MakeOp(51, OpType::Xor, {a, c});
    auto expr = MakeOp(52, OpType::Xor, {x1, x2});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_Xor_Cancel_Rule());
    engine.AddRule(Factorize::Get_Xor_Pair_Cancel_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::Xor);
    BF_TEST(result->inputs.size() == 2);

    auto i0 = result->inputs[0];
    auto i1 = result->inputs[1];

    BF_TEST((i0->id == b->id && i1->id == c->id) || (i0->id == c->id && i1->id == b->id));
    return 0;
}

int main() {
    BF_RUN_TEST(TestXorXorCancelPair);
    return 0;
}