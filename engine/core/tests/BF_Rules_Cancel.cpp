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

int main() {
    BF_RUN_TEST(TestXorCancel);
    return 0;
}