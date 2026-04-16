#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <Core_Expr.h>
#include <TestAssert.h>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Testing;

namespace {

uint64_t RunRuntimeCExpr(const std::string& cExpr) {
    namespace fs = std::filesystem;

    static uint64_t counter = 0;
    const fs::path base = fs::temp_directory_path() /
                          ("bf_codegen_runtime_" + std::to_string(static_cast<unsigned long long>(::getpid())) + "_" +
                           std::to_string(counter++));
    const fs::path srcPath = base;
    const fs::path exePath = base.string() + ".out";

    {
        std::ofstream src(srcPath);
        src << "#include <cstdint>\n"
               "#include <iostream>\n"
               "int main() {\n"
               "    uint64_t value = static_cast<uint64_t>("
            << cExpr
            << ");\n"
               "    std::cout << value;\n"
               "    return 0;\n"
               "}\n";
    }

    const char* cxx = std::getenv("CXX");
    const std::string compiler = (cxx && *cxx) ? cxx : "c++";
    const std::string compileCmd = compiler + " -std=c++20 -O0 \"" + srcPath.string() + "\" -o \"" +
                                   exePath.string() + "\"";
    BF_TEST(std::system(compileCmd.c_str()) == 0);

    const std::string runCmd = "\"" + exePath.string() + "\"";
    FILE* pipe = popen(runCmd.c_str(), "r");
    BF_TEST(pipe != nullptr);

    char buffer[128] = {0};
    std::string output;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        output += buffer;

    BF_TEST(pclose(pipe) == 0);

    fs::remove(srcPath);
    fs::remove(exePath);

    return static_cast<uint64_t>(std::stoull(output));
}

void AssertEvalMatchesRuntime(const AST::Expr* expr, uint32_t bitWidth) {
    const Eval::EvalResult eval = Eval::EvaluateConstant(expr, bitWidth);
    BF_TEST(eval.status == Eval::EvalStatus::Success);

    const std::string cExpr = Codegen::EmitCExpr(expr, bitWidth);
    const uint64_t runtime = RunRuntimeCExpr(cExpr);

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
