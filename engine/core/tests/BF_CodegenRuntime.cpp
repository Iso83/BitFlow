#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <Core_Expr.h>
#include <TestAssert.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Testing;

namespace {

#if defined(_WIN32)
#define BF_POPEN _popen
#define BF_PCLOSE _pclose
#else
#define BF_POPEN popen
#define BF_PCLOSE pclose
#endif

static uint64_t CompileAndRun(const std::string& expr) {
    const char* file = "bf_codegen_test.cpp";
    const char* exe = "bf_codegen_test.exe";

    std::ofstream out(file);
    out << "#include <cstdint>\n";
    out << "#include <iostream>\n";
    out << "int main(){\n";
    out << "uint64_t v = " << expr << ";\n";
    out << "std::cout << v;\n";
    out << "return 0;\n";
    out << "}\n";
    out.close();

    int res = std::system("g++ -std=c++20 bf_codegen_test.cpp -o bf_codegen_test.exe");
    if (res != 0)
        return 0;

#if defined(_WIN32)
    const char* runCmd = exe;
#else
    const char* runCmd = "./bf_codegen_test.exe";
#endif

    FILE* pipe = BF_POPEN(runCmd, "r");
    if (!pipe)
        return 0;

    char buffer[128] = {0};
    if (!fgets(buffer, sizeof(buffer), pipe)) {
        BF_PCLOSE(pipe);
        return 0;
    }

    BF_PCLOSE(pipe);
    std::remove(file);
    std::remove(exe);

    return static_cast<uint64_t>(std::stoull(buffer));
}

int TestCodegenRuntime_Case1_SimpleAdd() {
    auto a = MakeConst(1, 10);
    auto b = MakeConst(2, 20);
    auto expr = MakeOp(3, OpType::Add, {a, b});

    auto eval = Eval::EvaluateConstant(expr, 32);
    BF_TEST(eval.status == Eval::EvalStatus::Success);

    auto code = Codegen::EmitCExpr(expr, 32);
    for (int i = 0; i < 10; ++i)
        BF_TEST(eval.value == CompileAndRun(code));
    return 0;
}

int TestCodegenRuntime_Case2_NestedMulAdd() {
    auto a = MakeConst(10, 3);
    auto b = MakeConst(11, 4);
    auto c = MakeConst(12, 5);

    auto expr = MakeOp(13, OpType::Mul, {MakeOp(14, OpType::Add, {a, b}), c});

    auto eval = Eval::EvaluateConstant(expr, 32);
    BF_TEST(eval.status == Eval::EvalStatus::Success);

    auto code = Codegen::EmitCExpr(expr, 32);
    for (int i = 0; i < 10; ++i)
        BF_TEST(eval.value == CompileAndRun(code));
    return 0;
}

int TestCodegenRuntime_Case3_Bitwise() {
    auto a = MakeConst(20, 0xF0);
    auto b = MakeConst(21, 0xAA);
    auto c = MakeConst(22, 0x3C);

    auto expr = MakeOp(23, OpType::And, {MakeOp(24, OpType::Xor, {a, b}), c});

    auto eval = Eval::EvaluateConstant(expr, 32);
    BF_TEST(eval.status == Eval::EvalStatus::Success);

    auto code = Codegen::EmitCExpr(expr, 32);
    for (int i = 0; i < 10; ++i)
        BF_TEST(eval.value == CompileAndRun(code));
    return 0;
}

int TestCodegenRuntime_Case4_RotateLeft() {
    auto a = MakeConst(30, 0x12345678);
    auto s = MakeConst(31, 5);
    auto expr = MakeOp(32, OpType::RotL, {a, s});

    auto eval = Eval::EvaluateConstant(expr, 32);
    BF_TEST(eval.status == Eval::EvalStatus::Success);

    auto code = Codegen::EmitCExpr(expr, 32);
    for (int i = 0; i < 10; ++i)
        BF_TEST(eval.value == CompileAndRun(code));
    return 0;
}

int TestCodegenRuntime_Case5_MaskingOverflow8Bit() {
    auto a = MakeConst(40, 250);
    auto b = MakeConst(41, 20);
    auto c = MakeConst(42, 15);

    auto addOverflow = MakeOp(43, OpType::Add, {a, b});
    auto mulOverflow = MakeOp(44, OpType::Mul, {addOverflow, c});

    auto eval = Eval::EvaluateConstant(mulOverflow, 8);
    BF_TEST(eval.status == Eval::EvalStatus::Success);

    auto code = Codegen::EmitCExpr(mulOverflow, 8);
    for (int i = 0; i < 10; ++i)
        BF_TEST(eval.value == CompileAndRun(code));
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestCodegenRuntime_Case1_SimpleAdd);
    BF_RUN_TEST(TestCodegenRuntime_Case2_NestedMulAdd);
    BF_RUN_TEST(TestCodegenRuntime_Case3_Bitwise);
    BF_RUN_TEST(TestCodegenRuntime_Case4_RotateLeft);
    BF_RUN_TEST(TestCodegenRuntime_Case5_MaskingOverflow8Bit);
    return 0;
}
