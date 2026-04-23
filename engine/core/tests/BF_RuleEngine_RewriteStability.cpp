#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <RewriteTestHelpers.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::Tests;

int TestSimplifyProfileRunWithInfoIsStable() {
    RuleEngine engine = BuildProfile(RuleProfile::simplify_full_safe);

    Expr* x = MakeVar(1);
    Expr* expr = MakeOp(100, OpType::Xor, {x, x});

    const RewriteResult info = engine.RunWithInfo(expr);
    BF_TEST(RewriteStable(info));
    BF_TEST(RewriteHasNoCycle(info));
    BF_TEST(RewriteWithinIterationLimit(info, 8));
    BF_TEST(info.result->isConst());
    BF_TEST(info.result->constValue == 0U);
    return 0;
}

int TestFactorizeProfileRunWithInfoIsStable() {
    RuleEngine engine = BuildProfile(RuleProfile::factorize_full_safe);

    Expr* a = MakeVar(1);
    Expr* b = MakeVar(2);
    Expr* c = MakeVar(3);
    Expr* expr = MakeOp(200, OpType::Add, {MakeOp(201, OpType::Mul, {a, b}), MakeOp(202, OpType::Mul, {a, c})});

    const RewriteResult info = engine.RunWithInfo(expr);
    BF_TEST(RewriteStableWithoutCycleWithin(info, 8));
    BF_TEST(info.result->op == OpType::Mul);
    return 0;
}

int TestExploreProfileRunWithInfoIsStable() {
    RuleEngine engine = BuildProfile(RuleProfile::explore);

    Expr* x = MakeVar(11);
    Expr* y = MakeVar(12);
    Expr* expr = MakeOp(300, OpType::And, {x, y});

    const RewriteResult info = engine.RunWithInfo(expr);
    BF_TEST(RewriteStable(info));
    BF_TEST(RewriteHasNoCycle(info));
    BF_TEST(RewriteWithinIterationLimit(info, 4));
    BF_TEST(info.result->op == OpType::And);
    return 0;
}

int main() {
    BF_RUN_TEST(TestSimplifyProfileRunWithInfoIsStable);
    BF_RUN_TEST(TestFactorizeProfileRunWithInfoIsStable);
    BF_RUN_TEST(TestExploreProfileRunWithInfoIsStable);
    return 0;
}
