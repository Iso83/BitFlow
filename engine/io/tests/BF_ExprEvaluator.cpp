#include <BitFlow/io/ExprEvaluator.h>
#include <TestAssert.h>
#include <string>

using Status = BitFlow::Core::Eval::EvalStatus;

static int ExpectValue(const char* expr, uint32_t bitWidth, uint64_t expectedValue) {
    const auto result = BitFlow::IO::ParseEvaluatePrint(expr, bitWidth);
    BF_TEST(result.eval.status == Status::Success);
    BF_TEST(result.eval.value == expectedValue);
    BF_TEST(result.text ==
            ("result: success, value=" + std::to_string(expectedValue) + ", bitwidth=" + std::to_string(bitWidth)));
    BF_TEST(result.parseOk);
    BF_TEST(result.parseError.empty());
    return 0;
}

int TestExprEvaluator_ConstantExpression() {
    BF_RUN_TEST(ExpectValue, "1 + 2 * 3", 8, 7);
    return 0;
}

int TestExprEvaluator_NotConstantExpression() {
    auto result = BitFlow::IO::ParseEvaluatePrint("a + 1", 8);

    BF_TEST(result.eval.status == Status::NotConstant);
    BF_TEST(result.text == "result: error: expression is not fully constant");
    BF_TEST(result.parseOk);
    return 0;
}

int TestExprEvaluator_DivisionAndModuloByZero() {
    auto divResult = BitFlow::IO::ParseEvaluatePrint("7 / 0", 8);
    auto modResult = BitFlow::IO::ParseEvaluatePrint("7 % 0", 8);

    BF_TEST(divResult.eval.status == Status::DivisionByZero);
    BF_TEST(divResult.text == "result: error: division by zero");
    BF_TEST(divResult.parseOk);

    BF_TEST(modResult.eval.status == Status::ModuloByZero);
    BF_TEST(modResult.text == "result: error: modulo by zero");
    BF_TEST(modResult.parseOk);
    return 0;
}

int TestExprEvaluator_RotateModuloBehavior() {
    // 8-bit: rotl(0b10010001, 9) == rotl(..., 1) == 0b00100011 (35)
    BF_RUN_TEST(ExpectValue, "rotl(145, 9)", 8, 35);
    // 16-bit: rotl(1, 20) => rotl(1, 4) => 16
    BF_RUN_TEST(ExpectValue, "rotl(1, 20)", 16, 16);
    return 0;
}

int TestExprEvaluator_ShiftEdgeCases() {
    // shift 0
    BF_RUN_TEST(ExpectValue, "179 << 0", 8, 179);
    // shift == bitwidth => 0
    BF_RUN_TEST(ExpectValue, "179 << 8", 8, 0);
    BF_RUN_TEST(ExpectValue, "179 >> 8", 8, 0);
    // shift > bitwidth => 0
    BF_RUN_TEST(ExpectValue, "179 << 9", 8, 0);
    BF_RUN_TEST(ExpectValue, "179 >>> 9", 8, 0);
    return 0;
}

int TestExprEvaluator_MultipleBitWidthsAndMasking() {
    // (255 + 1) masked by width
    BF_RUN_TEST(ExpectValue, "255 + 1", 8, 0);
    BF_RUN_TEST(ExpectValue, "255 + 1", 16, 256);
    BF_RUN_TEST(ExpectValue, "255 + 1", 32, 256);
    BF_RUN_TEST(ExpectValue, "255 + 1", 64, 256);

    // verify intermediate masking: ((255 + 1) * 3)
    // 8-bit: (255 + 1)=0, then 0*3=0
    BF_RUN_TEST(ExpectValue, "(255 + 1) * 3", 8, 0);
    // 16-bit: (255 + 1)=256, then 256*3=768
    BF_RUN_TEST(ExpectValue, "(255 + 1) * 3", 16, 768);
    return 0;
}

int TestExprEvaluator_InvalidBitWidth() {
    auto above = BitFlow::IO::ParseEvaluatePrint("7", 65);
    auto zero = BitFlow::IO::ParseEvaluatePrint("7", 0);

    BF_TEST(above.eval.status == Status::InvalidBitWidth);
    BF_TEST(above.text == "result: error: invalid bitwidth (must be in range 1..64)");
    BF_TEST(above.parseOk);

    BF_TEST(zero.eval.status == Status::InvalidBitWidth);
    BF_TEST(zero.text == "result: error: invalid bitwidth (must be in range 1..64)");
    BF_TEST(zero.parseOk);
    return 0;
}

int TestExprEvaluator_UnsupportedOp() {
    const auto result = BitFlow::IO::ParseEvaluatePrint("-5", 8);
    BF_TEST(result.eval.status == Status::UnsupportedOp);
    BF_TEST(result.text == "result: error: unsupported operation for constant evaluation");
    BF_TEST(result.parseOk);
    return 0;
}

int TestExprEvaluator_ParseFailure() {
    const auto result = BitFlow::IO::ParseEvaluatePrint("rotl(1)", 8);
    BF_TEST(!result.parseOk);
    BF_TEST(result.eval.status == Status::NotConstant);
    BF_TEST(result.text.find("result: error: parse failed:") == 0);
    BF_TEST(!result.parseError.empty());
    return 0;
}

int main() {
    BF_RUN_TEST(TestExprEvaluator_ConstantExpression);
    BF_RUN_TEST(TestExprEvaluator_NotConstantExpression);
    BF_RUN_TEST(TestExprEvaluator_DivisionAndModuloByZero);
    BF_RUN_TEST(TestExprEvaluator_RotateModuloBehavior);
    BF_RUN_TEST(TestExprEvaluator_ShiftEdgeCases);
    BF_RUN_TEST(TestExprEvaluator_MultipleBitWidthsAndMasking);
    BF_RUN_TEST(TestExprEvaluator_InvalidBitWidth);
    BF_RUN_TEST(TestExprEvaluator_UnsupportedOp);
    BF_RUN_TEST(TestExprEvaluator_ParseFailure);
    return 0;
}
