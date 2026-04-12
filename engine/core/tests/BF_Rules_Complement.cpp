#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestAndComplement() {
    auto a = MakeVar(1);
    auto na = MakeOp(2, OpType::Not, {a});

    auto expr = MakeOp(3, OpType::And, {a, na});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_Idempotent_Rule());
    engine.AddRule(Simplify::Get_Complement_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->isConst);
    BF_TEST(r->constValue == 0);
    return 0;
}

int TestOrComplement() {
    auto a = MakeVar(1);
    auto na = MakeOp(2, OpType::Not, {a});

    auto expr = MakeOp(3, OpType::Or, {a, na});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_Idempotent_Rule());
    engine.AddRule(Simplify::Get_Complement_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->isConst);
    BF_TEST(r->constValue == 1);
    return 0;
}

int main() {
    BF_RUN_TEST(TestAndComplement);
    BF_RUN_TEST(TestOrComplement);
    return 0;
}