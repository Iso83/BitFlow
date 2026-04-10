#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestAndIdempotent() {
    auto a = MakeVar(1, OpType::And);

    auto expr = MakeOp(10, OpType::And, {a, a});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_Idempotent_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->id == a->id);
    return 0;
}

int TestOrIdempotent() {
    auto a = MakeVar(1, OpType::Or);

    auto expr = MakeOp(10, OpType::Or, {a, a});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_Idempotent_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->id == a->id);
    return 0;
}

int main() {
    BF_RUN_TEST(TestAndIdempotent);
    BF_RUN_TEST(TestOrIdempotent);
    return 0;
}