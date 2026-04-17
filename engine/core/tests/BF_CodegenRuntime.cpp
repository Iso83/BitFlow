#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <utility>

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
    static std::atomic<uint64_t> counter{0};
    const uint64_t id = counter.fetch_add(1);

    const std::string base = "bf_codegen_test_" + std::to_string(id);
    const std::string file = base + ".cpp";

#if defined(_WIN32)
    const std::string exe = base + ".exe";
#else
    const std::string exe = base;
#endif

    std::ofstream out(file);
    out << "#include <cstdint>\n";
    out << "#include <iostream>\n";
    out << "int main(){\n";
    out << "uint64_t v = " << expr << ";\n";
    out << "std::cout << v;\n";
    out << "return 0;\n";
    out << "}\n";
    out.close();

    int res = 0;

    const std::string gppCmd = "g++ -std=c++20 " + file + " -o " + exe;
    res = std::system(gppCmd.c_str());

    if (res != 0) {
        const std::string cxxCmd = "c++ -std=c++20 " + file + " -o " + exe;
        res = std::system(cxxCmd.c_str());
    }

#if defined(_WIN32)
    if (res != 0) {
        // fallback --> MSVC
        const std::string clCmd = "cl /nologo /std:c++20 /O2 /EHsc " + file + " /Fe:" + exe;
        res = std::system(clCmd.c_str());
    }
#endif

    if (res != 0) {
        std::remove(file.c_str());
        return 0;
    }

#if defined(_WIN32)
    const std::string runCmd = exe;
#else
    const std::string runCmd = "./" + exe;
#endif

    FILE* pipe = BF_POPEN(runCmd.c_str(), "r");
    if (!pipe) {
        std::remove(file.c_str());
        std::remove(exe.c_str());
#if defined(_WIN32)
        std::remove((base + ".obj").c_str());
#endif
        return 0;
    }

    char buffer[128] = {0};
    if (!fgets(buffer, sizeof(buffer), pipe)) {
        BF_PCLOSE(pipe);
        std::remove(file.c_str());
        std::remove(exe.c_str());
#if defined(_WIN32)
        std::remove((base + ".obj").c_str());
#endif
        return 0;
    }

    BF_PCLOSE(pipe);

    std::remove(file.c_str());
    std::remove(exe.c_str());
#if defined(_WIN32)
    std::remove((base + ".obj").c_str());
#endif

    return static_cast<uint64_t>(std::stoull(buffer));
}

static uint64_t CompileAndRunWrapper(const std::string& wrapper, const std::string& invocation) {
    static std::atomic<uint64_t> counter{100000};
    const uint64_t id = counter.fetch_add(1);

    const std::string base = "bf_codegen_wrapper_test_" + std::to_string(id);
    const std::string file = base + ".cpp";

#if defined(_WIN32)
    const std::string exe = base + ".exe";
#else
    const std::string exe = base;
#endif

    std::ofstream out(file);
    out << "#include <cstdint>\n";
    out << "#include <iostream>\n";
    out << wrapper << "\n";
    out << "int main(){\n";
    out << "std::cout << " << invocation << ";\n";
    out << "return 0;\n";
    out << "}\n";
    out.close();

    int res = 0;

    const std::string gppCmd = "g++ -std=c++20 " + file + " -o " + exe;
    res = std::system(gppCmd.c_str());

    if (res != 0) {
        const std::string cxxCmd = "c++ -std=c++20 " + file + " -o " + exe;
        res = std::system(cxxCmd.c_str());
    }

#if defined(_WIN32)
    if (res != 0) {
        const std::string clCmd = "cl /nologo /std:c++20 /O2 /EHsc " + file + " /Fe:" + exe;
        res = std::system(clCmd.c_str());
    }
#endif

    if (res != 0) {
        std::remove(file.c_str());
        return 0;
    }

#if defined(_WIN32)
    const std::string runCmd = exe;
#else
    const std::string runCmd = "./" + exe;
#endif

    FILE* pipe = BF_POPEN(runCmd.c_str(), "r");
    if (!pipe) {
        std::remove(file.c_str());
        std::remove(exe.c_str());
#if defined(_WIN32)
        std::remove((base + ".obj").c_str());
#endif
        return 0;
    }

    char buffer[128] = {0};
    if (!fgets(buffer, sizeof(buffer), pipe)) {
        BF_PCLOSE(pipe);
        std::remove(file.c_str());
        std::remove(exe.c_str());
#if defined(_WIN32)
        std::remove((base + ".obj").c_str());
#endif
        return 0;
    }

    BF_PCLOSE(pipe);
    std::remove(file.c_str());
    std::remove(exe.c_str());
#if defined(_WIN32)
    std::remove((base + ".obj").c_str());
#endif

    return static_cast<uint64_t>(std::stoull(buffer));
}

static std::pair<uint64_t, uint64_t> CompileAndRunMultiWrapper(const std::string& wrapper,
                                                               const std::string& invocation) {
    static std::atomic<uint64_t> counter{200000};
    const uint64_t id = counter.fetch_add(1);

    const std::string base = "bf_codegen_multi_wrapper_test_" + std::to_string(id);
    const std::string file = base + ".cpp";

#if defined(_WIN32)
    const std::string exe = base + ".exe";
#else
    const std::string exe = base;
#endif

    std::ofstream out(file);
    out << "#include <cstdint>\n";
    out << "#include <iostream>\n";
    out << wrapper << "\n";
    out << "int main(){\n";
    out << "auto r = " << invocation << ";\n";
    out << "std::cout << r.out1 << \",\" << r.out2;\n";
    out << "return 0;\n";
    out << "}\n";
    out.close();

    int res = 0;
    const std::string gppCmd = "g++ -std=c++20 " + file + " -o " + exe;
    res = std::system(gppCmd.c_str());

    if (res != 0) {
        const std::string cxxCmd = "c++ -std=c++20 " + file + " -o " + exe;
        res = std::system(cxxCmd.c_str());
    }

#if defined(_WIN32)
    if (res != 0) {
        const std::string clCmd = "cl /nologo /std:c++20 /O2 /EHsc " + file + " /Fe:" + exe;
        res = std::system(clCmd.c_str());
    }
#endif

    if (res != 0) {
        std::remove(file.c_str());
        return {0ull, 0ull};
    }

#if defined(_WIN32)
    const std::string runCmd = exe;
#else
    const std::string runCmd = "./" + exe;
#endif

    FILE* pipe = BF_POPEN(runCmd.c_str(), "r");
    if (!pipe) {
        std::remove(file.c_str());
        std::remove(exe.c_str());
#if defined(_WIN32)
        std::remove((base + ".obj").c_str());
#endif
        return {0ull, 0ull};
    }

    char buffer[256] = {0};
    if (!fgets(buffer, sizeof(buffer), pipe)) {
        BF_PCLOSE(pipe);
        std::remove(file.c_str());
        std::remove(exe.c_str());
#if defined(_WIN32)
        std::remove((base + ".obj").c_str());
#endif
        return {0ull, 0ull};
    }

    BF_PCLOSE(pipe);
    std::remove(file.c_str());
    std::remove(exe.c_str());
#if defined(_WIN32)
    std::remove((base + ".obj").c_str());
#endif

    const std::string raw = buffer;
    const size_t comma = raw.find(',');
    if (comma == std::string::npos)
        return {0ull, 0ull};

    const uint64_t first = static_cast<uint64_t>(std::stoull(raw.substr(0, comma)));
    const uint64_t second = static_cast<uint64_t>(std::stoull(raw.substr(comma + 1)));
    return {first, second};
}

int TestCodegenRuntime_Case1_SimpleAdd() {
    auto a = MakeConst(1, 10);
    auto b = MakeConst(2, 20);
    auto expr = MakeOp(3, OpType::Add, {a, b});

    auto eval = Eval::EvaluateConstant(expr, 32);
    if (eval.status != Eval::EvalStatus::Success)
        return 0;

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
    if (eval.status != Eval::EvalStatus::Success)
        return 0;

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
    if (eval.status != Eval::EvalStatus::Success)
        return 0;

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
    if (eval.status != Eval::EvalStatus::Success)
        return 0;

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
    if (eval.status != Eval::EvalStatus::Success)
        return 0;

    auto code = Codegen::EmitCExpr(mulOverflow, 8);
    for (int i = 0; i < 10; ++i)
        BF_TEST(eval.value == CompileAndRun(code));
    return 0;
}

int TestCodegenRuntime_Case6_FunctionWrapperInvocation() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto expr = MakeOp(50, OpType::Add, {a, b});

    auto wrapper = Codegen::EmitCFunction(expr, 32);
    const uint64_t result = CompileAndRunWrapper(wrapper, "f(5, 7)");

    BF_TEST(result == 12ull);
    return 0;
}

int TestCodegenRuntime_Case7_MultiOutputConstantOnly() {
    auto a = MakeConst(60, 10);
    auto b = MakeConst(61, 20);
    auto c = MakeConst(62, 7);

    auto expr1 = MakeOp(63, OpType::Add, {a, b});
    auto expr2 = MakeOp(64, OpType::Xor, {MakeOp(65, OpType::Add, {a, b}), c});

    auto eval1 = Eval::EvaluateConstant(expr1, 32);
    auto eval2 = Eval::EvaluateConstant(expr2, 32);
    if (eval1.status != Eval::EvalStatus::Success || eval2.status != Eval::EvalStatus::Success)
        return 0;

    const std::vector<const AST::Expr*> outputs = {expr1, expr2};
    const auto wrapper = Codegen::EmitCFunctionMulti(outputs, 32);
    const auto [out1, out2] = CompileAndRunMultiWrapper(wrapper, "f()");

    BF_TEST(out1 == eval1.value);
    BF_TEST(out2 == eval2.value);
    return 0;
}

int TestCodegenRuntime_Case8_MultiOutputStructuralIdenticalSeparateSubtrees() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);
    auto d = MakeVar(4);

    auto x1 = MakeOp(70, OpType::Xor, {a, b});
    auto x2 = MakeOp(71, OpType::Xor, {a, b}); // apart Expr*, wel structureel identiek
    auto outExpr1 = MakeOp(72, OpType::Add, {x1, c});
    auto outExpr2 = MakeOp(73, OpType::And, {x2, d});

    const std::vector<const AST::Expr*> outputs = {outExpr1, outExpr2};
    const auto wrapper = Codegen::EmitCFunctionMulti(outputs, 32);
    const auto [out1, out2] = CompileAndRunMultiWrapper(wrapper, "f(9, 5, 3, 6)");

    BF_TEST(out1 == 15ull); // ((9 ^ 5) + 3)
    BF_TEST(out2 == 4ull);  // ((9 ^ 5) & 6)
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestCodegenRuntime_Case1_SimpleAdd);
    BF_RUN_TEST(TestCodegenRuntime_Case2_NestedMulAdd);
    BF_RUN_TEST(TestCodegenRuntime_Case3_Bitwise);
    BF_RUN_TEST(TestCodegenRuntime_Case4_RotateLeft);
    BF_RUN_TEST(TestCodegenRuntime_Case5_MaskingOverflow8Bit);
    BF_RUN_TEST(TestCodegenRuntime_Case6_FunctionWrapperInvocation);
    BF_RUN_TEST(TestCodegenRuntime_Case7_MultiOutputConstantOnly);
    BF_RUN_TEST(TestCodegenRuntime_Case8_MultiOutputStructuralIdenticalSeparateSubtrees);
    return 0;
}
