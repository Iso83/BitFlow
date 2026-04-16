#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Testing;

int main() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);

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

    // Case 1 — precedence add/mul: (a + b) * c
    auto mulOverAddExpr = MakeOp(17, OpType::Mul, {MakeOp(18, OpType::Add, {a, b}), c});
    auto mulOverAddCode = Codegen::EmitCExpr(mulOverAddExpr, 32);
    BF_TEST(mulOverAddCode ==
            "(((((((((v1) & ((1ull << 32) - 1ull)) + ((v2) & ((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull))) * "
            "((v3) & ((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull))");

    // Case 2 — shift met add rechts: a << (b + c)
    auto shlAddExpr = MakeOp(19, OpType::Shl, {a, MakeOp(20, OpType::Add, {b, c})});
    auto shlAddCode = Codegen::EmitCExpr(shlAddExpr, 32);
    BF_TEST(shlAddCode ==
            "((((((v1) & ((1ull << 32) - 1ull)) << ((((((v2) & ((1ull << 32) - 1ull)) + ((v3) & ((1ull << 32) - "
            "1ull))) & ((1ull << 32) - 1ull))) % 32ull)) & ((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull))");

    // Case 3 — unary boven binary: ~(a ^ b)
    auto notXorExpr = MakeOp(21, OpType::Not, {MakeOp(22, OpType::Xor, {a, b})});
    auto notXorCode = Codegen::EmitCExpr(notXorExpr, 32);
    BF_TEST(notXorCode ==
            "((((~(((((v1) & ((1ull << 32) - 1ull)) ^ ((v2) & ((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull)))) & "
            "((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull))");

    // Case 4 — nested bitwise: (a ^ b) & c
    auto andOverXorExpr = MakeOp(23, OpType::And, {MakeOp(24, OpType::Xor, {a, b}), c});
    auto andOverXorCode = Codegen::EmitCExpr(andOverXorExpr, 32);
    BF_TEST(andOverXorCode ==
            "(((((((((v1) & ((1ull << 32) - 1ull)) ^ ((v2) & ((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull))) & "
            "((v3) & ((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull))");

    // Case 5 — canonical mask vorm (32/64 bit)
    BF_TEST(Codegen::EmitCExpr(a, 32).find("((1ull << 32) - 1ull)") != std::string::npos);
    BF_TEST(Codegen::EmitCExpr(a, 64) == "((((v1) & 0xffffffffffffffffull)) & 0xffffffffffffffffull)");

    return 0;
}
