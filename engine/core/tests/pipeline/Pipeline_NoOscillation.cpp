#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <RewriteTestHelpers.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Tests;

namespace {

int AssertStableNoCycle(RuleEngine& engine, Expr* expr, int maxIterations) {
    const RewriteResult info = engine.RewriteToFixedPoint(expr);
    BF_TEST(RewriteStable(info));
    BF_TEST(RewriteHasNoCycle(info));
    BF_TEST(RewriteWithinIterationLimit(info, maxIterations));
    return 0;
}

int Test_NoOscillation_BitwiseCommonFactor() {
    Expr* a = MakeVar(1);
    Expr* b = MakeVar(2);
    Expr* c = MakeVar(3);
    Expr* expr = MakeOp(100, OpType::Xor, {MakeOp(101, OpType::And, {a, b}), MakeOp(102, OpType::And, {a, c})});

    RuleEngine engine = BuildProfile("factorize_full_safe");
    BF_RUN_TEST(AssertStableNoCycle, engine, expr, 8);
    return 0;
}

int Test_NoOscillation_AndXor_WithoutDistribute() {
    Expr* a = MakeVar(10);
    Expr* b = MakeVar(11);
    Expr* c = MakeVar(12);
    Expr* expr = MakeOp(200, OpType::And, {a, MakeOp(201, OpType::Xor, {b, c})});

    RuleEngine engine = BuildProfile("factorize_full_safe");
    BF_RUN_TEST(AssertStableNoCycle, engine, expr, 6);
    return 0;
}

int Test_NoOscillation_AndXor_WithDistribute() {
    Expr* a = MakeVar(20);
    Expr* b = MakeVar(21);
    Expr* c = MakeVar(22);
    Expr* expr = MakeOp(300, OpType::And, {a, MakeOp(301, OpType::Xor, {b, c})});

    RuleEngine engine = BuildProfile("expand_bitwise");
    BF_RUN_TEST(AssertStableNoCycle, engine, expr, 8);
    return 0;
}

int Test_NoOscillation_ArithmeticCommonFactor() {
    Expr* a = MakeVar(30);
    Expr* b = MakeVar(31);
    Expr* c = MakeVar(32);
    Expr* expr = MakeOp(400, OpType::Add, {MakeOp(401, OpType::Mul, {a, b}), MakeOp(402, OpType::Mul, {a, c})});

    RuleEngine engine = BuildProfile("factorize_arithmetic_safe");
    BF_RUN_TEST(AssertStableNoCycle, engine, expr, 8);
    return 0;
}

int Test_NoOscillation_ArithmeticGuardCase() {
    Expr* a = MakeVar(40);
    Expr* b = MakeVar(41);
    Expr* expr = MakeOp(500, OpType::Add, {a, MakeOp(501, OpType::Mul, {a, b})});

    RuleEngine engine = BuildProfile("factorize_arithmetic_safe");
    BF_RUN_TEST(AssertStableNoCycle, engine, expr, 8);
    return 0;
}

int Test_NoOscillation_ArithmeticMultiplicity() {
    Expr* a = MakeVar(50);
    Expr* expr =
        MakeOp(600, OpType::Add,
               {a, MakeOp(601, OpType::Mul, {MakeConst(603, 2), a}), MakeOp(602, OpType::Mul, {MakeConst(604, 3), a})});

    RuleEngine engine = BuildProfile("factorize_arithmetic_safe");
    BF_RUN_TEST(AssertStableNoCycle, engine, expr, 8);
    return 0;
}

int Test_NoOscillation_FactorizeThenExpand_PipelineCombo() {
    Expr* a = MakeVar(60);
    Expr* b = MakeVar(61);
    Expr* c = MakeVar(62);
    Expr* distributed = MakeOp(700, OpType::Xor, {MakeOp(701, OpType::And, {a, b}), MakeOp(702, OpType::And, {a, c})});

    RuleEngine factorizeEngine = BuildProfile("factorize_bitwise_safe");
    const RewriteResult factorized = factorizeEngine.RewriteToFixedPoint(distributed);
    BF_TEST(RewriteStableWithoutCycleWithin(factorized, 8));

    RuleEngine expandEngine = BuildProfile("expand_bitwise");
    BF_RUN_TEST(AssertStableNoCycle, expandEngine, factorized.result, 8);
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(Test_NoOscillation_BitwiseCommonFactor);
    BF_RUN_TEST(Test_NoOscillation_AndXor_WithoutDistribute);
    BF_RUN_TEST(Test_NoOscillation_AndXor_WithDistribute);
    BF_RUN_TEST(Test_NoOscillation_ArithmeticCommonFactor);
    BF_RUN_TEST(Test_NoOscillation_ArithmeticGuardCase);
    BF_RUN_TEST(Test_NoOscillation_ArithmeticMultiplicity);
    BF_RUN_TEST(Test_NoOscillation_FactorizeThenExpand_PipelineCombo);
    return 0;
}
