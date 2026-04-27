#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestAndIdempotent() {
    auto a = MakeVar(1);

    auto expr = MakeOp(10, OpType::And, {a, a});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->id == a->id);
    return 0;
}

int TestAndIdempotentMixed() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);

    auto expr = MakeOp(11, OpType::And, {a, b, a});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::And);
    BF_TEST(r->inputs.size() == 2);
    BF_TEST(r->inputs[0]->id == a->id);
    BF_TEST(r->inputs[1]->id == b->id);
    return 0;
}

int TestOrIdempotent() {
    auto a = MakeVar(1);

    auto expr = MakeOp(20, OpType::Or, {a, a});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->id == a->id);
    return 0;
}

int TestOrIdempotentMixed() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);

    auto expr = MakeOp(21, OpType::Or, {b, a, b});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::Or);
    BF_TEST(r->inputs.size() == 2);
    BF_TEST(r->inputs[0]->id == b->id);
    BF_TEST(r->inputs[1]->id == a->id);
    return 0;
}

int TestAndIdempotentFrozen() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);

    auto expr = MakeOp(30, OpType::And, {a, b, a});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r != expr);
    BF_TEST(r->op == OpType::And);
    BF_TEST(r->inputs.size() == 2);
    BF_TEST(r->inputs[0]->id == a->id);
    BF_TEST(r->inputs[1]->id == b->id);
    return 0;
}

int main() {
    BF_RUN_TEST(TestAndIdempotent);
    BF_RUN_TEST(TestAndIdempotentMixed);
    BF_RUN_TEST(TestOrIdempotent);
    BF_RUN_TEST(TestOrIdempotentMixed);
    BF_RUN_TEST(TestAndIdempotentFrozen);
    return 0;
}