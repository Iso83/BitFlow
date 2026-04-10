#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestAndCancelPair() {
    auto x = MakeVar(1, OpType::And);

    auto expr = MakeOp(2, OpType::And, {x, x});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_And_Cancel_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->id == x->id);
    return 0;
}

int TestAndCancelMixed() {
    auto x = MakeVar(1, OpType::And);
    auto y = MakeVar(2, OpType::And);

    auto expr = MakeOp(3, OpType::And, {x, y, x});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_And_Cancel_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::And);
    BF_TEST(result->inputs.size() == 3);
    BF_TEST(result->inputs[0]->id != result->inputs[1]->id);
    return 0;
}

int TestOrCancelPair() {
    auto x = MakeVar(1, OpType::Or);

    auto expr = MakeOp(2, OpType::Or, {x, x});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Get_Or_Cancel_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->id == x->id);
    return 0;
}

int TestOrCancelMixed() {
    auto x = MakeVar(1, OpType::Or);
    auto y = MakeVar(2, OpType::Or);

    auto expr = MakeOp(3, OpType::Or, {y, x, y});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Get_Or_Cancel_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::Or);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id != result->inputs[1]->id);
    return 0;
}

int TestXorCancel() {
    auto x = MakeVar(1, OpType::Xor);
    auto y = MakeVar(2, OpType::Xor);

    auto expr = MakeOp(3, OpType::Xor, {x, x, y});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_Xor_Cancel_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->id == y->id);
    return 0;
}

int TestXorCancelMulti() {
    auto a = MakeVar(1, OpType::Xor);
    auto b = MakeVar(2, OpType::Xor);
    auto c = MakeVar(3, OpType::Xor);

    auto expr = MakeOp(10, OpType::Xor, {a, b, c, a});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_Xor_Cancel_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::Xor);
    BF_TEST(r->inputs.size() == 2);

    return 0;
}

int TestXorDuplicateCancelPair() {
    auto a = MakeVar(1, OpType::Xor);

    auto expr = MakeOp(2, OpType::Xor, {a, a});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_Xor_DuplicateCancel_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->isConst);
    BF_TEST(r->constValue == 0);
    return 0;
}

int TestXorDuplicateCancelMixed() {
    auto a = MakeVar(1, OpType::Xor);
    auto b = MakeVar(2, OpType::Xor);

    auto expr = MakeOp(3, OpType::Xor, {a, b, a});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_Xor_DuplicateCancel_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->id == b->id);
    return 0;
}

int TestXorDuplicateCancelAll() {
    auto a = MakeVar(1, OpType::Xor);
    auto b = MakeVar(2, OpType::Xor);

    auto expr = MakeOp(3, OpType::Xor, {a, b, a, b});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Get_Xor_DuplicateCancel_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->isConst);
    BF_TEST(r->constValue == 0);
    return 0;
}

int main() {
    BF_RUN_TEST(TestAndCancelPair);
    BF_RUN_TEST(TestAndCancelMixed);
    BF_RUN_TEST(TestOrCancelPair);
    BF_RUN_TEST(TestOrCancelMixed);
    BF_RUN_TEST(TestXorCancel);
    BF_RUN_TEST(TestXorCancelMulti);
    BF_RUN_TEST(TestXorDuplicateCancelPair);
    BF_RUN_TEST(TestXorDuplicateCancelMixed);
    BF_RUN_TEST(TestXorDuplicateCancelAll);
    return 0;
}