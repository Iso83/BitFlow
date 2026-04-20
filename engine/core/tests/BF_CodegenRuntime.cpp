#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

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

static int CompileCppSource(const std::string& file, const std::string& exe) {
    const std::string strictFlags = " -Wall -Wextra -Wpedantic -Werror";
    int res = std::system(("g++ -std=c++20" + strictFlags + " " + file + " -o " + exe).c_str());
    if (res != 0)
        res = std::system(("c++ -std=c++20" + strictFlags + " " + file + " -o " + exe).c_str());
#if defined(_WIN32)
    if (res != 0)
        res = std::system(("cl /nologo /std:c++20 /O2 /W4 /WX /EHsc " + file + " /Fe:" + exe).c_str());
#endif
    return res;
}

static uint64_t CompileAndRun(const std::string& expr, const std::string& support = {}) {
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
    out << support;
    if (!support.empty() && support.back() != '\n')
        out << "\n";
    out << "#include <iostream>\n";
    out << "int main(){\n";
    out << "uint64_t v = " << expr << ";\n";
    out << "std::cout << v;\n";
    out << "return 0;\n";
    out << "}\n";
    out.close();

    int res = CompileCppSource(file, exe);
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

static uint64_t CompileAndRunWrapper(const std::string& wrapper, const std::string& invocation,
                                     const std::string& support = {}) {
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
    out << support;
    if (!support.empty() && support.back() != '\n')
        out << "\n";
    out << wrapper << "\n";
    out << "#include <iostream>\n";
    out << "int main(){\n";
    out << "std::cout << " << invocation << ";\n";
    out << "return 0;\n";
    out << "}\n";
    out.close();

    int res = CompileCppSource(file, exe);
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

} // namespace

int main() {
    // 32-bit generated expression compile path.
    auto c1 = MakeConst(1, 10);
    auto c2 = MakeConst(2, 20);
    auto expr = MakeOp(3, OpType::Add, {c1, c2});
    auto eval = Eval::EvaluateConstant(expr, 32);
    BF_TEST(eval.status == Eval::EvalStatus::Success);
    BF_TEST(eval.value == CompileAndRun(Codegen::EmitCExpr(expr, 32), Codegen::EmitCRuntimeSupport(32)));

    // 32-bit generated function compile path.
    auto a = MakeVar(11);
    auto b = MakeVar(12);
    auto wrapperExpr = MakeOp(13, OpType::Mul, {MakeOp(14, OpType::Add, {a, b}), MakeConst(15, 2)});
    auto wrapper = Codegen::EmitCFunction(wrapperExpr, 32);
    BF_TEST(CompileAndRunWrapper(wrapper, "f(5, 7)", Codegen::EmitCRuntimeSupport(32)) == 24ull);

    // Rotate usage path should compile/run with explicit support contract.
    auto rotlExpr = MakeOp(16, OpType::RotL, {MakeConst(17, 0x81), MakeConst(18, 1)});
    auto rotlEval = Eval::EvaluateConstant(rotlExpr, 8);
    BF_TEST(rotlEval.status == Eval::EvalStatus::Success);
    BF_TEST(rotlEval.value == CompileAndRun(Codegen::EmitCExpr(rotlExpr, 8), Codegen::EmitCRuntimeSupport(8)));

    auto rotArg = MakeVar(19);
    auto rotFnExpr = MakeOp(20, OpType::RotL, {rotArg, MakeConst(21, 1)});
    auto rotFn = Codegen::EmitCFunction(rotFnExpr, 32);
    BF_TEST(CompileAndRunWrapper(rotFn, "f(0x81u)", Codegen::EmitCRuntimeSupport(32)) == 0x102ull);

    // 64-bit generated function path.
    auto x64 = MakeVar(30);
    auto y64 = MakeVar(31);
    auto fn64Expr = MakeOp(32, OpType::Add, {x64, y64});
    auto fn64 = Codegen::EmitCFunction(fn64Expr, 64);
    const uint64_t a64 = 0x123456789abcdef0ull;
    const uint64_t b64 = 0x1111111111111111ull;
    BF_TEST(CompileAndRunWrapper(fn64, "f(0x123456789abcdef0ull, 0x1111111111111111ull)",
                                 Codegen::EmitCRuntimeSupport(64)) == static_cast<uint64_t>(a64 + b64));

    // Multi-output generated function path.
    auto mA = MakeVar(40);
    auto mB = MakeVar(41);
    auto out1 = MakeOp(42, OpType::Add, {mA, mB});
    auto out2 = MakeOp(43, OpType::Xor, {mA, mB});
    std::vector<const AST::Expr*> outputs = {out1, out2};
    auto multiFn = Codegen::EmitCFunctionMulti(outputs, 32);
    const auto support32 = Codegen::EmitCRuntimeSupport(32);
    BF_TEST(CompileAndRunWrapper(multiFn, "f(5u, 7u).out1", support32) == 12ull);
    BF_TEST(CompileAndRunWrapper(multiFn, "f(5u, 7u).out2", support32) == (5ull ^ 7ull));

    return 0;
}
