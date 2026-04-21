#include <BitFlow/core/ast/ExprStruct.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

namespace {

int AssertSameCanonicalOutput(RuleEngine& a, Expr* inputA, RuleEngine& b, Expr* inputB) {
    const RewriteResult outA = a.RunWithInfo(inputA);
    const RewriteResult outB = b.RunWithInfo(inputB);

    BF_TEST(outA.stable);
    BF_TEST(!outA.cycleDetected());
    BF_TEST(outB.stable);
    BF_TEST(!outB.cycleDetected());
    BF_TEST(BitFlow::Core::AST::StructEqual(outA.result, outB.result));
    return 0;
}

int TestManualEqualsProfile_ShaSafe() {
    RuleEngine profileEngine = BuildProfile("sha_safe");

    RuleEngine manualEngine;
    Add_Normalize_Rules(manualEngine);
    Add_Simplify_Bitwise_Rules(manualEngine);
    Add_Simplify_Arithmetic_Rules(manualEngine);
    Add_Simplify_SHA_Rules(manualEngine);

    Expr* a0 = MakeVar(1);
    Expr* b0 = MakeVar(2);
    Expr* c0 = MakeVar(3);
    Expr* d0 = MakeVar(4);
    Expr* e0 = MakeVar(5);
    Expr* inProfile =
        MakeOp(10, OpType::Xor, {MakeOp(11, OpType::Ch, {a0, b0, c0}), MakeOp(12, OpType::Maj, {a0, d0, e0})});

    Expr* a1 = MakeVar(1);
    Expr* b1 = MakeVar(2);
    Expr* c1 = MakeVar(3);
    Expr* d1 = MakeVar(4);
    Expr* e1 = MakeVar(5);
    Expr* inManual =
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

    Expr* a0 = MakeVar(20);
    Expr* b0 = MakeVar(21);
    Expr* c0 = MakeVar(22);
    Expr* inProfile = MakeOp(23, OpType::Xor, {MakeOp(24, OpType::And, {a0, b0}), MakeOp(25, OpType::And, {a0, c0})});

    Expr* a1 = MakeVar(20);
    Expr* b1 = MakeVar(21);
    Expr* c1 = MakeVar(22);
    Expr* inManual = MakeOp(26, OpType::Xor, {MakeOp(27, OpType::And, {a1, b1}), MakeOp(28, OpType::And, {a1, c1})});

    BF_RUN_TEST(AssertSameCanonicalOutput, profileEngine, inProfile, manualEngine, inManual);
    return 0;
}

int TestManualEqualsProfile_FactorizeArithmeticSafe() {
    RuleEngine profileEngine = BuildProfile("factorize_arithmetic_safe");

    RuleEngine manualEngine;
    Add_Normalize_Rules(manualEngine);
    Add_Factorize_Arithmetic_Safe_Rules(manualEngine);

    Expr* a0 = MakeVar(40);
    Expr* b0 = MakeVar(41);
    Expr* c0 = MakeVar(42);
    Expr* inProfile = MakeOp(43, OpType::Add, {MakeOp(44, OpType::Mul, {a0, b0}), MakeOp(45, OpType::Mul, {a0, c0})});

    Expr* a1 = MakeVar(40);
    Expr* b1 = MakeVar(41);
    Expr* c1 = MakeVar(42);
    Expr* inManual = MakeOp(46, OpType::Add, {MakeOp(47, OpType::Mul, {a1, b1}), MakeOp(48, OpType::Mul, {a1, c1})});

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
