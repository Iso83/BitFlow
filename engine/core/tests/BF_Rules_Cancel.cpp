#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestXorCancel() {
    auto x = MakeVar(1, OpType::Xor);
    auto y = MakeVar(2, OpType::Xor);

    auto expr = MakeOp(3, OpType::Xor, {x, x, y});

    RuleEngine engine;
    engine.AddRule(Simplify::Get_Xor_Cancel_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->id == y->id);
    return 0;
}

int TestXorCancelNary() {
    auto a = MakeVar(1, OpType::Xor);
    auto b = MakeVar(2, OpType::Xor);
    auto c = MakeVar(3, OpType::Xor);

    auto expr = MakeOp(4, OpType::Xor, {a, b, a, c});

    RuleEngine engine;
    engine.AddRule(Simplify::Get_Xor_Cancel_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::Xor);
    BF_TEST(result->inputs.size() == 2);

    auto i0 = result->inputs[0];
    auto i1 = result->inputs[1];

    BF_TEST((i0->id == b->id && i1->id == c->id) || (i0->id == c->id && i1->id == b->id));
    return 0;
}

int main() {
    BF_RUN_TEST(TestXorCancel);
    BF_RUN_TEST(TestXorCancelNary);
    return 0;
}
