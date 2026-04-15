#include <BitFlow/core/eval/ConstEvaluator.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <iostream>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Eval;

static int ExpectSuccess(Expr* expr, uint32_t bitWidth, uint64_t expected) {
    EvalResult result = EvaluateConstant(expr, bitWidth);
    BF_TEST(result.status == EvalStatus::Success);
    if (result.value != expected) {
        std::cerr << "Expected " << expected << ", got " << result.value << " at bitWidth " << bitWidth << std::endl;
        return -1;
    }
    return 0;
}

int TestInvalidBitWidth() {
    auto c = MakeConst(1, 7);

    BF_TEST(EvaluateConstant(c, 0).status == EvalStatus::InvalidBitWidth);
    BF_TEST(EvaluateConstant(c, 65).status == EvalStatus::InvalidBitWidth);
    return 0;
}

int TestLiteralAndMasking() {
    auto c = MakeConst(1, 0xFFu);
    BF_RUN_TEST(ExpectSuccess, c, 4, 0xFu);
    BF_RUN_TEST(ExpectSuccess, c, 8, 0xFFu);
    return 0;
}

int TestArithmetic() {
    auto a = MakeConst(1, 14);
    auto b = MakeConst(2, 5);
    auto c = MakeConst(3, 6);

    auto add = MakeOp(10, OpType::Add, {a, b, c});
    auto sub = MakeOp(11, OpType::Sub, {a, b});
    auto mul = MakeOp(12, OpType::Mul, {a, b, c});

    BF_RUN_TEST(ExpectSuccess, add, 4, 9);  // 14 + 5 + 6 = 25 -> 9
    BF_RUN_TEST(ExpectSuccess, sub, 4, 9);  // 14 - 5 = 9
    BF_RUN_TEST(ExpectSuccess, mul, 4, 4);  // 14*5*6 = 420 -> 4
    return 0;
}

int TestBitwiseAndNot() {
    auto a = MakeConst(1, 0b10101100u);
    auto b = MakeConst(2, 0b00111100u);
    auto c = MakeConst(3, 0b01010101u);

    auto andExpr = MakeOp(10, OpType::And, {a, b, c});
    auto orExpr = MakeOp(11, OpType::Or, {a, b, c});
    auto xorExpr = MakeOp(12, OpType::Xor, {a, b, c});
    auto notExpr = MakeOp(13, OpType::Not, {a});

    BF_RUN_TEST(ExpectSuccess, andExpr, 8, 0b00000100u);
    BF_RUN_TEST(ExpectSuccess, orExpr, 8, 0b11111101u);
    BF_RUN_TEST(ExpectSuccess, xorExpr, 8, 0b11000101u);
    BF_RUN_TEST(ExpectSuccess, notExpr, 8, 0b01010011u);
    return 0;
}

int TestLogicalShifts() {
    auto lhs = MakeConst(1, 0b10110011u);
    auto zero = MakeConst(2, 0);
    auto three = MakeConst(3, 3);
    auto eight = MakeConst(4, 8);
    auto nine = MakeConst(5, 9);

    BF_RUN_TEST(ExpectSuccess, MakeOp(10, OpType::Shl, {lhs, zero}), 8, 0b10110011u);
    BF_RUN_TEST(ExpectSuccess, MakeOp(11, OpType::Shl, {lhs, three}), 8, 0b10011000u);
    BF_RUN_TEST(ExpectSuccess, MakeOp(12, OpType::Shl, {lhs, eight}), 8, 0u);
    BF_RUN_TEST(ExpectSuccess, MakeOp(13, OpType::Shl, {lhs, nine}), 8, 0u);

    BF_RUN_TEST(ExpectSuccess, MakeOp(14, OpType::Shr, {lhs, zero}), 8, 0b10110011u);
    BF_RUN_TEST(ExpectSuccess, MakeOp(15, OpType::Shr, {lhs, three}), 8, 0b00010110u);
    BF_RUN_TEST(ExpectSuccess, MakeOp(16, OpType::Shr, {lhs, eight}), 8, 0u);
    BF_RUN_TEST(ExpectSuccess, MakeOp(17, OpType::Shr, {lhs, nine}), 8, 0u);

    BF_RUN_TEST(ExpectSuccess, MakeOp(18, OpType::UShr, {lhs, three}), 8, 0b00010110u);
    BF_RUN_TEST(ExpectSuccess, MakeOp(19, OpType::UShr, {lhs, eight}), 8, 0u);
    return 0;
}

int TestRotates() {
    auto lhs = MakeConst(1, 0b10010001u);
    auto zero = MakeConst(2, 0);
    auto three = MakeConst(3, 3);
    auto eight = MakeConst(4, 8);
    auto nine = MakeConst(5, 9);

    BF_RUN_TEST(ExpectSuccess, MakeOp(10, OpType::RotL, {lhs, zero}), 8, 0b10010001u);
    BF_RUN_TEST(ExpectSuccess, MakeOp(11, OpType::RotL, {lhs, three}), 8, 0b10001100u);
    BF_RUN_TEST(ExpectSuccess, MakeOp(12, OpType::RotL, {lhs, eight}), 8, 0b10010001u);
    BF_RUN_TEST(ExpectSuccess, MakeOp(13, OpType::RotL, {lhs, nine}), 8, 0b00100011u);

    BF_RUN_TEST(ExpectSuccess, MakeOp(14, OpType::RotR, {lhs, three}), 8, 0b00110010u);
    BF_RUN_TEST(ExpectSuccess, MakeOp(15, OpType::RotR, {lhs, eight}), 8, 0b10010001u);
    BF_RUN_TEST(ExpectSuccess, MakeOp(16, OpType::RotR, {lhs, nine}), 8, 0b11001000u);
    return 0;
}

int TestDivAndMod() {
    auto seven = MakeConst(1, 7);
    auto two = MakeConst(2, 2);
    auto zero = MakeConst(3, 0);

    BF_RUN_TEST(ExpectSuccess, MakeOp(10, OpType::Div, {seven, two}), 4, 3);
    BF_RUN_TEST(ExpectSuccess, MakeOp(11, OpType::Mod, {seven, two}), 4, 1);

    BF_TEST(EvaluateConstant(MakeOp(12, OpType::Div, {seven, zero}), 8).status == EvalStatus::DivisionByZero);
    BF_TEST(EvaluateConstant(MakeOp(13, OpType::Mod, {seven, zero}), 8).status == EvalStatus::ModuloByZero);
    return 0;
}

int TestNotConstantAndUnsupported() {
    auto x = MakeVar(1);
    auto c3 = MakeConst(2, 3);

    BF_TEST(EvaluateConstant(MakeOp(10, OpType::Add, {x, c3}), 8).status == EvalStatus::NotConstant);

    EvalResult negResult = EvaluateConstant(MakeOp(11, OpType::Neg, {c3}), 8);
    BF_TEST(negResult.status == EvalStatus::UnsupportedOp);
    BF_TEST(negResult.unsupportedOp == OpType::Neg);

    EvalResult chResult = EvaluateConstant(MakeOp(12, OpType::Ch, {c3, c3, c3}), 8);
    BF_TEST(chResult.status == EvalStatus::UnsupportedOp);
    BF_TEST(chResult.unsupportedOp == OpType::Ch);
    return 0;
}

int TestBitwidth64() {
    auto c0 = MakeConst(1, 0);
    auto c1 = MakeConst(2, 1);
    auto c31 = MakeConst(3, 31);

    // Ensure mask and rotate logic work at width=64 (without relying on signed behavior).
    BF_RUN_TEST(ExpectSuccess, MakeOp(10, OpType::Not, {c0}), 64, ~uint64_t{0});
    BF_RUN_TEST(ExpectSuccess, MakeOp(11, OpType::RotL, {c1, c31}), 64, uint64_t{1} << 31);
    return 0;
}

int main() {
    BF_RUN_TEST(TestInvalidBitWidth);
    BF_RUN_TEST(TestLiteralAndMasking);
    BF_RUN_TEST(TestArithmetic);
    BF_RUN_TEST(TestBitwiseAndNot);
    BF_RUN_TEST(TestLogicalShifts);
    BF_RUN_TEST(TestRotates);
    BF_RUN_TEST(TestDivAndMod);
    BF_RUN_TEST(TestNotConstantAndUnsupported);
    BF_RUN_TEST(TestBitwidth64);
    return 0;
}
