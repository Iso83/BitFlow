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

void AssertEvalMatchesRuntime(const AST::Expr* expr, uint32_t bitWidth) {
    const Eval::EvalResult eval = Eval::EvaluateConstant(expr, bitWidth);
    BF_TEST(eval.status == Eval::EvalStatus::Success);

    const std::string cExpr = Codegen::EmitCExpr(expr, bitWidth);
    const uint64_t runtime = CompileAndRun(cExpr);

    BF_TEST(runtime == eval.value);
}

int TestCodegenRuntime_MatchesEvaluator() {
    auto c7 = MakeConst(1, 7);
    auto c13 = MakeConst(2, 13);
    auto c3 = MakeConst(3, 3);
    auto c5 = MakeConst(4, 5);
    auto c40 = MakeConst(5, 40);
    auto c9 = MakeConst(6, 9);

    auto addMul = MakeOp(7, OpType::Mul, {MakeOp(8, OpType::Add, {c7, c13}), c3});
    auto bitMix = MakeOp(9, OpType::Xor, {MakeOp(10, OpType::And, {c40, c13}), MakeOp(11, OpType::Or, {c9, c5})});
    auto shifts = MakeOp(12, OpType::Add, {MakeOp(13, OpType::Shl, {c13, c40}), MakeOp(14, OpType::Shr, {c40, c3})});
    auto rotate = MakeOp(15, OpType::Xor, {MakeOp(16, OpType::RotL, {c40, c5}), MakeOp(17, OpType::RotR, {c40, c3})});
    auto unary = MakeOp(18, OpType::Add, {MakeOp(19, OpType::Not, {c40}), MakeOp(20, OpType::Neg, {c13})});

    AssertEvalMatchesRuntime(addMul, 8);
    AssertEvalMatchesRuntime(addMul, 16);
    AssertEvalMatchesRuntime(bitMix, 16);
    AssertEvalMatchesRuntime(shifts, 32);
    AssertEvalMatchesRuntime(rotate, 32);
    AssertEvalMatchesRuntime(unary, 32);

    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestCodegenRuntime_MatchesEvaluator);
    return 0;
}
