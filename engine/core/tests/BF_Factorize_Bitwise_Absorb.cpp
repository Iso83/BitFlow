#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestAndAbsorb() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);

    auto inner = MakeOp(3, OpType::Or, {a, b});
    auto expr = MakeOp(4, OpType::And, {a, inner});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Factorize::Bitwise::Get_And_Absorb_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->id == a->id);
    return 0;
}

int TestOrAbsorb() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);

    auto inner = MakeOp(3, OpType::And, {a, b});
    auto expr = MakeOp(4, OpType::Or, {a, inner});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Factorize::Bitwise::Get_Or_Absorb_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->id == a->id);
    return 0;
}

int main() {
    BF_RUN_TEST(TestAndAbsorb);
    BF_RUN_TEST(TestOrAbsorb);
    return 0;
}