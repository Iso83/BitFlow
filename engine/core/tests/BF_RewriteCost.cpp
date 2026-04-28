#include <BitFlow/core/rules/RewriteCost.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::Core::Expression;

int TestRewriteCostCapturesCoreShapeSignals() {
    ExprOld* a = MakeVar(1);
    ExprOld* b = MakeVar(2);
    ExprOld* c = MakeVar(3);
    ExprOld* d = MakeVar(4);

    ExprOld* factored = MakeOp(100, OpType::And, {a, MakeOp(101, OpType::Xor, {b, c})});
    ExprOld* distributed =
        MakeOp(200, OpType::Xor, {MakeOp(201, OpType::And, {a, b}), MakeOp(202, OpType::And, {a, c})});
    ExprOld* nestedAssoc = MakeOp(300, OpType::Xor, {a, MakeOp(301, OpType::Xor, {b, c, d})});

    const RewriteCost factoredCost = ComputeRewriteCost(factored);
    const RewriteCost distributedCost = ComputeRewriteCost(distributed);
    const RewriteCost nestedCost = ComputeRewriteCost(nestedAssoc);

    BF_TEST(factoredCost.distributivePatterns == 1U);
    BF_TEST(distributedCost.distributivePatterns == 0U);
    BF_TEST(nestedCost.nestedAssociativeNodes >= 1U);
    BF_TEST(distributedCost.totalNodes > factoredCost.totalNodes);
    BF_TEST(distributedCost.operatorNodes > factoredCost.operatorNodes);
    return 0;
}

int TestRewriteCostPoliciesAreDeterministic() {
    ExprOld* a = MakeVar(10);
    ExprOld* b = MakeVar(11);
    ExprOld* c = MakeVar(12);

    ExprOld* factored = MakeOp(400, OpType::And, {a, MakeOp(401, OpType::Xor, {b, c})});
    ExprOld* distributed =
        MakeOp(500, OpType::Xor, {MakeOp(501, OpType::And, {a, b}), MakeOp(502, OpType::And, {a, c})});

    BF_TEST(IsRewritePreferred(factored, distributed, RewriteCostPolicy::FactorizeSafe));
    BF_TEST(IsRewritePreferred(distributed, factored, RewriteCostPolicy::ExpandDistribute));
    return 0;
}

int main() {
    BF_RUN_TEST(TestRewriteCostCapturesCoreShapeSignals);
    BF_RUN_TEST(TestRewriteCostPoliciesAreDeterministic);
    return 0;
}
