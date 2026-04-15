#include <BitFlow/io/ExprEvaluator.h>
#include <TestAssert.h>

int TestExprEvaluator_ParseEvaluatePrintSuccess() {
    auto result = BitFlow::IO::ParseEvaluatePrint("1 + 2 * 3", 8);

    BF_TEST(result.eval.status == BitFlow::Core::Eval::EvalStatus::Success);
    BF_TEST(result.eval.value == 7);
    BF_TEST(result.text == "7");
    return 0;
}

int TestExprEvaluator_ParseEvaluatePrintNotConstant() {
    auto result = BitFlow::IO::ParseEvaluatePrint("a + 1", 8);

    BF_TEST(result.eval.status == BitFlow::Core::Eval::EvalStatus::NotConstant);
    BF_TEST(result.text == "error: expression is not fully constant");
    return 0;
}

int TestExprEvaluator_ParseEvaluatePrintDivisionByZero() {
    auto result = BitFlow::IO::ParseEvaluatePrint("7 / 0", 8);

    BF_TEST(result.eval.status == BitFlow::Core::Eval::EvalStatus::DivisionByZero);
    BF_TEST(result.text == "error: division by zero");
    return 0;
}

int TestExprEvaluator_ParseEvaluatePrintInvalidBitWidth() {
    auto result = BitFlow::IO::ParseEvaluatePrint("7", 65);
    auto resultZero = BitFlow::IO::ParseEvaluatePrint("7", 0);

    BF_TEST(result.eval.status == BitFlow::Core::Eval::EvalStatus::InvalidBitWidth);
    BF_TEST(result.text == "error: invalid bitwidth (must be in range 1..64)");
    BF_TEST(resultZero.eval.status == BitFlow::Core::Eval::EvalStatus::InvalidBitWidth);
    BF_TEST(resultZero.text == "error: invalid bitwidth (must be in range 1..64)");
    return 0;
}

int main() {
    BF_RUN_TEST(TestExprEvaluator_ParseEvaluatePrintSuccess);
    BF_RUN_TEST(TestExprEvaluator_ParseEvaluatePrintNotConstant);
    BF_RUN_TEST(TestExprEvaluator_ParseEvaluatePrintDivisionByZero);
    BF_RUN_TEST(TestExprEvaluator_ParseEvaluatePrintInvalidBitWidth);
    return 0;
}
