#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <ProfileEngines.h>
#include <SHA_Expr.h>
#include <TestAssert.h>
#include <string>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Testing::SHA;

namespace {

RuleEngine MakeSigmaNormalizeEngine() {
    return MakeShaSafeEngine();
}

std::string ExprSignature(const Expr* root) {
    if (!root)
        return "null";

    std::string sig = std::to_string(static_cast<int>(root->op)) + ":" + std::to_string(root->constValue) + "[";
    for (size_t i = 0; i < root->inputs.size(); ++i) {
        if (i != 0)
            sig += ",";
        sig += ExprSignature(root->inputs[i]);
    }
    sig += "]";
    return sig;
}

int TestBigSigma0_PermutationCanonicalizes() {
    Builder b;
    auto x = b.Var();
    auto a = b.Xor({b.RotR(x, 2), b.RotR(x, 13), b.RotR(x, 22)});
    auto c = b.Xor({b.RotR(x, 22), b.RotR(x, 2), b.RotR(x, 13)});

    auto ra = MakeSigmaNormalizeEngine().ApplyUntilStable(a);
    auto rc = MakeSigmaNormalizeEngine().ApplyUntilStable(c);

    BF_TEST(ExprSignature(ra) == ExprSignature(rc));
    BF_TEST(Codegen::EmitCExpr(ra, 32).find("bf_rotr") != std::string::npos);
    return 0;
}

int TestBigSigma1_PermutationCanonicalizes() {
    Builder b;
    auto x = b.Var();
    auto a = b.Xor({b.RotR(x, 6), b.RotR(x, 11), b.RotR(x, 25)});
    auto c = b.Xor({b.RotR(x, 25), b.RotR(x, 6), b.RotR(x, 11)});

    auto ra = MakeSigmaNormalizeEngine().ApplyUntilStable(a);
    auto rc = MakeSigmaNormalizeEngine().ApplyUntilStable(c);

    BF_TEST(ExprSignature(ra) == ExprSignature(rc));
    BF_TEST(Codegen::EmitCExpr(ra, 32).find("bf_rotr") != std::string::npos);
    return 0;
}

int TestSmallSigma0_PermutationCanonicalizes() {
    Builder b;
    auto x = b.Var();
    auto a = b.Xor({b.RotR(x, 7), b.RotR(x, 18), MakeOp(17000, OpType::Shr, {x, b.Const(3)})});
    auto c = b.Xor({MakeOp(17001, OpType::Shr, {x, b.Const(3)}), b.RotR(x, 18), b.RotR(x, 7)});

    auto ra = MakeSigmaNormalizeEngine().ApplyUntilStable(a);
    auto rc = MakeSigmaNormalizeEngine().ApplyUntilStable(c);

    BF_TEST(ExprSignature(ra) == ExprSignature(rc));
    const auto emitted = Codegen::EmitCExpr(ra, 32);
    BF_TEST(emitted.find("bf_rotr") != std::string::npos);
    BF_TEST(emitted.find(">>") != std::string::npos);
    return 0;
}

int TestSmallSigma1_PermutationCanonicalizes() {
    Builder b;
    auto x = b.Var();
    auto a = b.Xor({b.RotR(x, 17), b.RotR(x, 19), MakeOp(17002, OpType::Shr, {x, b.Const(10)})});
    auto c = b.Xor({b.RotR(x, 19), MakeOp(17003, OpType::Shr, {x, b.Const(10)}), b.RotR(x, 17)});

    auto ra = MakeSigmaNormalizeEngine().ApplyUntilStable(a);
    auto rc = MakeSigmaNormalizeEngine().ApplyUntilStable(c);

    BF_TEST(ExprSignature(ra) == ExprSignature(rc));
    const auto emitted = Codegen::EmitCExpr(ra, 32);
    BF_TEST(emitted.find("bf_rotr") != std::string::npos);
    BF_TEST(emitted.find(">>") != std::string::npos);
    return 0;
}

int TestDuplicatePairCancelsToZero() {
    auto x = MakeVar(1);
    auto expr =
        MakeOp(20, OpType::Xor,
               {MakeOp(21, OpType::RotR, {x, MakeConst(22, 2)}), MakeOp(23, OpType::RotR, {x, MakeConst(24, 2)})});

    Expr* r = MakeSigmaNormalizeEngine().ApplyUntilStable(expr);
    BF_TEST(r->op == OpType::Const);
    BF_TEST(r->constValue == 0);
    return 0;
}

int TestDuplicateWithMiddleTermLeavesSingle() {
    auto x = MakeVar(1);
    auto keep = MakeOp(31, OpType::RotR, {x, MakeConst(32, 13)});
    auto expr = MakeOp(
        30, OpType::Xor,
        {MakeOp(33, OpType::RotR, {x, MakeConst(34, 2)}), keep, MakeOp(35, OpType::RotR, {x, MakeConst(36, 2)})});

    Expr* r = MakeSigmaNormalizeEngine().ApplyUntilStable(expr);
    BF_TEST(r == keep);
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestBigSigma0_PermutationCanonicalizes);
    BF_RUN_TEST(TestBigSigma1_PermutationCanonicalizes);
    BF_RUN_TEST(TestSmallSigma0_PermutationCanonicalizes);
    BF_RUN_TEST(TestSmallSigma1_PermutationCanonicalizes);
    BF_RUN_TEST(TestDuplicatePairCancelsToZero);
    BF_RUN_TEST(TestDuplicateWithMiddleTermLeavesSingle);
    return 0;
}
