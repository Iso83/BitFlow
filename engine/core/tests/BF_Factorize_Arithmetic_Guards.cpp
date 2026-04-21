#include <BitFlow/core/ast/ExprStruct.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

namespace {

int AssertNoRewrite(RuleEngine& engine, Expr* expr, int maxIterations = 6) {
    const RewriteResult info = engine.RunWithInfo(expr);
    BF_TEST(info.stable);
    BF_TEST(!info.cycleDetected());
    BF_TEST(info.iterations <= maxIterations);
    BF_TEST(BitFlow::Core::AST::StructEqual(info.result, expr));
    return 0;
}

int TestGuard_a_plus_a_mul_b_unchanged() {
    RuleEngine engine = BuildProfile("factorize_arithmetic_safe");

    Expr* a = MakeVar(1);
    Expr* b = MakeVar(2);
    Expr* expr = MakeOp(10, OpType::Add, {a, MakeOp(11, OpType::Mul, {a, b})});

    BF_RUN_TEST(AssertNoRewrite, engine, expr, 8);
    return 0;
}

int TestGuard_a_plus_a_shift_1_unchanged() {
    RuleEngine engine = BuildProfile("factorize_arithmetic_safe");

    Expr* a = MakeVar(20);
    Expr* expr = MakeOp(21, OpType::Add, {a, MakeOp(22, OpType::Shl, {a, MakeConst(23, 1)})});

    BF_RUN_TEST(AssertNoRewrite, engine, expr, 8);
    return 0;
}

int TestGuard_a_mul_b_plus_c_standalone_unchanged() {
    RuleEngine engine = BuildProfile("factorize_arithmetic_safe");

    Expr* a = MakeVar(30);
    Expr* b = MakeVar(31);
    Expr* c = MakeVar(32);
    Expr* expr = MakeOp(33, OpType::Mul, {a, MakeOp(34, OpType::Add, {b, c})});

    BF_RUN_TEST(AssertNoRewrite, engine, expr, 8);
    return 0;
}

int TestGuard_nonlinear_mix_unchanged() {
    RuleEngine engine = BuildProfile("factorize_arithmetic_safe");

    Expr* a = MakeVar(40);
    Expr* expr = MakeOp(41, OpType::Add, {a, MakeOp(42, OpType::Mul, {a, a})});

    BF_RUN_TEST(AssertNoRewrite, engine, expr, 8);
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestGuard_a_plus_a_mul_b_unchanged);
    BF_RUN_TEST(TestGuard_a_plus_a_shift_1_unchanged);
    BF_RUN_TEST(TestGuard_a_mul_b_plus_c_standalone_unchanged);
    BF_RUN_TEST(TestGuard_nonlinear_mix_unchanged);
    return 0;
}
