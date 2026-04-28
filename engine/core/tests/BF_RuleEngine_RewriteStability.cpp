#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <RewriteTestHelpers.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::Tests;

int TestSimplifyProfileRunWithInfoIsStable() {
    RuleEngine engine = BuildProfile(RuleProfile::simplify_full_safe);

    ExprOld* x = MakeVar(1);
    ExprOld* expr = MakeOp(100, OpType::Xor, {x, x});

    const RewriteResult info = engine.RewriteToFixedPoint(expr);
    BF_TEST(RewriteStable(info));
    BF_TEST(RewriteHasNoCycle(info));
    BF_TEST(RewriteWithinIterationLimit(info, 8));
    BF_TEST(info.result->op == OpType::Const);
    BF_TEST(info.result->constValue == 0U);
    return 0;
}

int TestFactorizeProfileRunWithInfoIsStable() {
    RuleEngine engine = BuildProfile(RuleProfile::factorize_full_safe);

    ExprOld* a = MakeVar(1);
    ExprOld* b = MakeVar(2);
    ExprOld* c = MakeVar(3);
    ExprOld* expr = MakeOp(200, OpType::Add, {MakeOp(201, OpType::Mul, {a, b}), MakeOp(202, OpType::Mul, {a, c})});

    const RewriteResult info = engine.RewriteToFixedPoint(expr);
    BF_TEST(RewriteStableWithoutCycleWithin(info, 8));
    BF_TEST(info.result->op == OpType::Mul);
    return 0;
}

int TestExploreProfileRunWithInfoIsStable() {
    RuleEngine engine = BuildProfile(RuleProfile::explore);

    ExprOld* a = MakeVar(11);
    ExprOld* b = MakeVar(12);
    ExprOld* c = MakeVar(13);
    ExprOld* expr = MakeOp(300, OpType::Xor, {MakeOp(301, OpType::And, {a, b}), MakeOp(302, OpType::And, {a, c})});

    const RewriteResult info = engine.RewriteToFixedPoint(expr);
    BF_TEST(RewriteStable(info));
    BF_TEST(RewriteHasNoCycle(info));
    BF_TEST(RewriteWithinIterationLimit(info, 8));
    BF_TEST(info.result->op == OpType::And || info.result->op == OpType::Xor);
    return 0;
}

int TestExpandBitwiseProfileRunWithInfoIsStable() {
    RuleEngine engine = BuildProfile(RuleProfile::expand_bitwise);

    ExprOld* a = MakeVar(21);
    ExprOld* b = MakeVar(22);
    ExprOld* c = MakeVar(23);
    ExprOld* expr = MakeOp(400, OpType::And, {a, MakeOp(401, OpType::Xor, {b, c})});

    const RewriteResult info = engine.RewriteToFixedPoint(expr);
    BF_TEST(RewriteStable(info));
    BF_TEST(RewriteHasNoCycle(info));
    BF_TEST(RewriteWithinIterationLimit(info, 8));
    BF_TEST(info.result->op == OpType::Xor);
    return 0;
}

int main() {
    BF_RUN_TEST(TestSimplifyProfileRunWithInfoIsStable);
    BF_RUN_TEST(TestFactorizeProfileRunWithInfoIsStable);
    BF_RUN_TEST(TestExploreProfileRunWithInfoIsStable);
    BF_RUN_TEST(TestExpandBitwiseProfileRunWithInfoIsStable);
    return 0;
}
