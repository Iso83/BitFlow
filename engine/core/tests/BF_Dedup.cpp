#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestXorDedup() {
    auto x = MakeVar(1, OpType::Xor);
    auto y = MakeVar(2, OpType::Xor);

    auto e1 = MakeOp(3, OpType::Xor, {x, y});
    auto e2 = MakeOp(4, OpType::Xor, {y, x});

    RuleEngine engine;
    Add_Bitwise_Simplify_Pipeline(engine);

    Expr* r1 = engine.ApplyUntilStable(e1);
    Expr* r2 = engine.ApplyUntilStable(e2);

    BF_TEST(r1 == r2);
    return 0;
}

int main() {
    BF_RUN_TEST(TestXorDedup);
    return 0;
}