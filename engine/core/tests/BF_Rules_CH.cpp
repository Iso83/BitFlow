#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestCH_Y_Y() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);

    auto a = MakeOp(10, OpType::And, {x, y});
    auto nx = MakeOp(11, OpType::Not, {x});
    auto b = MakeOp(12, OpType::And, {nx, y});

    auto expr = MakeOp(13, OpType::Xor, {a, b});

    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Bitwise_Rules(engine);
    Add_Simplify_SHA_Rules(engine);

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r == y);
    return 0;
}

int TestCH_X_X() {
    auto x = MakeVar(1);
    auto z = MakeVar(3);

    auto a = MakeOp(10, OpType::And, {x, x});
    auto nx = MakeOp(11, OpType::Not, {x});
    auto b = MakeOp(12, OpType::And, {nx, z});

    auto expr = MakeOp(13, OpType::Xor, {a, b});

    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Bitwise_Rules(engine);
    Add_Simplify_SHA_Rules(engine);

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::Or);
    BF_TEST(r->inputs.size() == 2);
    BF_TEST(r->inputs[0] == x);
    BF_TEST(r->inputs[1] == z);
    return 0;
}

int TestCH_Complement() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);

    auto ny = MakeOp(20, OpType::Not, {y});

    auto a = MakeOp(10, OpType::And, {x, y});
    auto nx = MakeOp(11, OpType::Not, {x});
    auto b = MakeOp(12, OpType::And, {nx, ny});

    auto expr = MakeOp(13, OpType::Xor, {a, b});

    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Bitwise_Rules(engine);
    Add_Simplify_SHA_Rules(engine);

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::Xor);
    return 0;
}

int main() {
    BF_RUN_TEST(TestCH_Y_Y);
    BF_RUN_TEST(TestCH_X_X);
    BF_RUN_TEST(TestCH_Complement);
    return 0;
}