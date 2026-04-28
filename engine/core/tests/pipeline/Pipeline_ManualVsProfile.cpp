#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

namespace {

int AssertSameCanonicalOutput(RuleEngine& a, ExprOld* inputA, RuleEngine& b, ExprOld* inputB) {
    const RewriteResult outA = a.RewriteToFixedPoint(inputA);
    const RewriteResult outB = b.RewriteToFixedPoint(inputB);

    BF_TEST(outA.stable);
    BF_TEST(!outA.cycleDetected());
    BF_TEST(outB.stable);
    BF_TEST(!outB.cycleDetected());
    BF_TEST(BitFlow::Core::Expression::StructEqual(outA.result, outB.result));
    return 0;
}

int TestManualEqualsProfile_ShaSafe() {
    RuleEngine profileEngine = BuildProfile("sha_safe");

    RuleEngine manualEngine;
    Add_Normalize_Rules(manualEngine);
    Add_Simplify_Bitwise_Rules(manualEngine);
    Add_Simplify_Arithmetic_Rules(manualEngine);
    Add_Simplify_SHA_Rules(manualEngine);

    ExprOld* a0 = MakeVar(1);
    ExprOld* b0 = MakeVar(2);
    ExprOld* c0 = MakeVar(3);
    ExprOld* d0 = MakeVar(4);
    ExprOld* e0 = MakeVar(5);
    ExprOld* inProfile =
        MakeOp(10, OpType::Xor, {MakeOp(11, OpType::Ch, {a0, b0, c0}), MakeOp(12, OpType::Maj, {a0, d0, e0})});

    ExprOld* a1 = MakeVar(1);
    ExprOld* b1 = MakeVar(2);
    ExprOld* c1 = MakeVar(3);
    ExprOld* d1 = MakeVar(4);
    ExprOld* e1 = MakeVar(5);
    ExprOld* inManual =
        MakeOp(13, OpType::Xor, {MakeOp(14, OpType::Ch, {a1, b1, c1}), MakeOp(15, OpType::Maj, {a1, d1, e1})});

    BF_RUN_TEST(AssertSameCanonicalOutput, profileEngine, inProfile, manualEngine, inManual);
    return 0;
}

int TestManualEqualsProfile_FactorizeBitwiseSafe() {
    RuleEngine profileEngine = BuildProfile("factorize_bitwise_safe");

    RuleEngine manualEngine;
    Add_Normalize_Rules(manualEngine);
    manualEngine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());
    Add_Factorize_Bitwise_Safe_Rules(manualEngine);

    ExprOld* a0 = MakeVar(20);
    ExprOld* b0 = MakeVar(21);
    ExprOld* c0 = MakeVar(22);
    ExprOld* inProfile =
        MakeOp(23, OpType::Xor, {MakeOp(24, OpType::And, {a0, b0}), MakeOp(25, OpType::And, {a0, c0})});

    ExprOld* a1 = MakeVar(20);
    ExprOld* b1 = MakeVar(21);
    ExprOld* c1 = MakeVar(22);
    ExprOld* inManual = MakeOp(26, OpType::Xor, {MakeOp(27, OpType::And, {a1, b1}), MakeOp(28, OpType::And, {a1, c1})});

    BF_RUN_TEST(AssertSameCanonicalOutput, profileEngine, inProfile, manualEngine, inManual);
    return 0;
}

int TestManualEqualsProfile_FactorizeArithmeticSafe() {
    RuleEngine profileEngine = BuildProfile("factorize_arithmetic_safe");

    RuleEngine manualEngine;
    Add_Normalize_Rules(manualEngine);
    Add_Factorize_Arithmetic_Safe_Rules(manualEngine);

    ExprOld* a0 = MakeVar(40);
    ExprOld* b0 = MakeVar(41);
    ExprOld* c0 = MakeVar(42);
    ExprOld* inProfile =
        MakeOp(43, OpType::Add, {MakeOp(44, OpType::Mul, {a0, b0}), MakeOp(45, OpType::Mul, {a0, c0})});

    ExprOld* a1 = MakeVar(40);
    ExprOld* b1 = MakeVar(41);
    ExprOld* c1 = MakeVar(42);
    ExprOld* inManual = MakeOp(46, OpType::Add, {MakeOp(47, OpType::Mul, {a1, b1}), MakeOp(48, OpType::Mul, {a1, c1})});

    BF_RUN_TEST(AssertSameCanonicalOutput, profileEngine, inProfile, manualEngine, inManual);
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestManualEqualsProfile_ShaSafe);
    BF_RUN_TEST(TestManualEqualsProfile_FactorizeBitwiseSafe);
    BF_RUN_TEST(TestManualEqualsProfile_FactorizeArithmeticSafe);
    return 0;
}
