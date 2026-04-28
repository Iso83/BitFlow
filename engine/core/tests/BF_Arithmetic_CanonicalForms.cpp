#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

static RuleEngine MakeArithmeticCanonicalEngine() {
    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Arithmetic_Rules(engine);
    Add_Factorize_Arithmetic_Rules(engine);
    return engine;
}

int TestCanonical_MulCoeffOrder_2a_plus_a() {
    ExprOld* a = MakeVar(1);
    ExprOld* expr = MakeOp(10, OpType::Add, {MakeOp(11, OpType::Mul, {MakeConst(12, 2), a}), a});

    RuleEngine engine = MakeArithmeticCanonicalEngine();
    ExprOld* out = engine.Rewrite(expr);

    BF_TEST(out->op == OpType::Mul);
    BF_TEST(out->inputs.size() == 2);
    BF_TEST(out->inputs[0]->id == a->id);
    BF_TEST(out->inputs[1]->op == OpType::Const);
    BF_TEST(out->inputs[1]->constValue == 3u);
    return 0;
}

int TestCanonical_a_b_plus_b_a() {
    ExprOld* a = MakeVar(20);
    ExprOld* b = MakeVar(21);
    ExprOld* expr = MakeOp(22, OpType::Add, {MakeOp(23, OpType::Mul, {a, b}), MakeOp(24, OpType::Mul, {b, a})});

    RuleEngine engine = MakeArithmeticCanonicalEngine();
    ExprOld* out = engine.Rewrite(expr);

    BF_TEST(out->op == OpType::Mul);
    BF_TEST(out->inputs.size() == 3);
    BF_TEST(out->inputs[0]->id == a->id);
    BF_TEST(out->inputs[1]->id == b->id);
    BF_TEST(out->inputs[2]->op == OpType::Const);
    BF_TEST(out->inputs[2]->constValue == 2u);
    return 0;
}

int TestCanonical_zero_mul_plus_a() {
    ExprOld* a = MakeVar(30);
    ExprOld* expr = MakeOp(31, OpType::Add, {MakeOp(32, OpType::Mul, {MakeConst(33, 0), a}), a});

    RuleEngine engine = MakeArithmeticCanonicalEngine();
    ExprOld* out = engine.Rewrite(expr);

    BF_TEST(out->id == a->id);
    return 0;
}

int TestCanonical_combineMulConstants_Order() {
    ExprOld* a = MakeVar(40);
    ExprOld* expr = MakeOp(41, OpType::Mul, {MakeConst(42, 2), a, MakeConst(43, 3)});

    RuleEngine engine = MakeArithmeticCanonicalEngine();
    ExprOld* out = engine.Rewrite(expr);

    BF_TEST(out->op == OpType::Mul);
    BF_TEST(out->inputs.size() == 2);
    BF_TEST(out->inputs[0]->id == a->id);
    BF_TEST(out->inputs[1]->op == OpType::Const);
    BF_TEST(out->inputs[1]->constValue == 6u);
    return 0;
}

int main() {
    BF_RUN_TEST(TestCanonical_MulCoeffOrder_2a_plus_a);
    BF_RUN_TEST(TestCanonical_a_b_plus_b_a);
    BF_RUN_TEST(TestCanonical_zero_mul_plus_a);
    BF_RUN_TEST(TestCanonical_combineMulConstants_Order);
    return 0;
}
