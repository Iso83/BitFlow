#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>

#include <tests/common/Core_Expr.h>
#include <tests/common/TestAssert.h>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Testing;

int main() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);

    auto addExpr = MakeOp(10, OpType::Add, {a, b});
    auto addCode = Codegen::EmitCExpr(addExpr, 32);

    BF_TEST(addCode.find("+") != std::string::npos);
    BF_TEST(addCode.find("((1ull << 32) - 1ull)") != std::string::npos);

    auto c1 = MakeConst(11, 1);
    auto c40 = MakeConst(12, 40);
    auto shlExpr = MakeOp(13, OpType::Shl, {c1, c40});

    auto shlEval = Eval::EvaluateConstant(shlExpr, 32);
    BF_TEST(shlEval.status == Eval::EvalStatus::Success);
    BF_TEST(shlEval.value == (1ull << (40ull % 32ull)));

    auto shlCode = Codegen::EmitCExpr(shlExpr, 32);
    BF_TEST(shlCode.find("% 32ull") != std::string::npos);

    auto rotExpr = MakeOp(14, OpType::RotL, {MakeConst(15, 0x12u), MakeConst(16, 33u)});
    auto rotEval = Eval::EvaluateConstant(rotExpr, 32);
    BF_TEST(rotEval.status == Eval::EvalStatus::Success);

    auto rotCode = Codegen::EmitCExpr(rotExpr, 32);
    BF_TEST(rotCode.find("% 32ull") != std::string::npos);
    BF_TEST(rotCode.find("((1ull << 32) - 1ull)") != std::string::npos);

    return 0;
}
