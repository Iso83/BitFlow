#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

static RuleEngine MakeArithmeticEngine() {
    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Arithmetic_Rules(engine);
    Add_Factorize_Arithmetic_Rules(engine);
    return engine;
}

int TestSimplify_NegNeg() {
    auto x = MakeVar(1);
    auto expr = MakeOp(2, OpType::Neg, {MakeOp(3, OpType::Neg, {x})});

    RuleEngine engine = MakeArithmeticEngine();
    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->id == x->id);
    return 0;
}

int TestSimplify_ConstCombine_Basic() {
    RuleEngine engine = MakeArithmeticEngine();

    auto add = MakeOp(10, OpType::Add, {MakeConst(11, 5), MakeConst(12, 7)});
    auto sub = MakeOp(13, OpType::Sub, {MakeConst(14, 9), MakeConst(15, 4)});
    auto mul = MakeOp(16, OpType::Mul, {MakeConst(17, 3), MakeConst(18, 6)});
    auto div = MakeOp(19, OpType::Div, {MakeConst(20, 8), MakeConst(21, 2)});

    BF_TEST(engine.ApplyUntilStable(add)->constValue == 12u);
    BF_TEST(engine.ApplyUntilStable(sub)->constValue == 5u);
    BF_TEST(engine.ApplyUntilStable(mul)->constValue == 18u);
    BF_TEST(engine.ApplyUntilStable(div)->constValue == 4u);
    return 0;
}

int TestModZero_Guard_Preserved() {
    auto x = MakeVar(30);
    auto mod = MakeOp(31, OpType::Mod, {x, MakeConst(32, 0)});

    RuleEngine engine = MakeArithmeticEngine();
    Expr* result = engine.ApplyUntilStable(mod);

    BF_TEST(result->op == OpType::Mod);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->isConst() && result->inputs[1]->constValue == 0u);
    return 0;
}

int TestFactorize_AddCommonFactor() {
    auto a = MakeVar(40);
    auto b = MakeVar(41);
    auto c = MakeVar(42);

    auto expr = MakeOp(43, OpType::Add, {MakeOp(44, OpType::Mul, {a, b}), MakeOp(45, OpType::Mul, {a, c})});

    RuleEngine engine = MakeArithmeticEngine();
    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::Mul);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id == a->id);
    BF_TEST(result->inputs[1]->op == OpType::Add);
    return 0;
}

int TestFactorize_Canonical_a_b_plus_b_a() {
    auto a = MakeVar(50);
    auto b = MakeVar(51);

    auto lhs = MakeOp(52, OpType::Mul, {a, b});
    auto rhs = MakeOp(53, OpType::Mul, {b, a});
    auto expr = MakeOp(54, OpType::Add, {lhs, rhs});

    RuleEngine engine = MakeArithmeticEngine();
    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::Mul);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[1]->op == OpType::Add);
    return 0;
}

int main() {
    BF_RUN_TEST(TestSimplify_NegNeg);
    BF_RUN_TEST(TestSimplify_ConstCombine_Basic);
    BF_RUN_TEST(TestModZero_Guard_Preserved);
    BF_RUN_TEST(TestFactorize_AddCommonFactor);
    BF_RUN_TEST(TestFactorize_Canonical_a_b_plus_b_a);
    return 0;
}
