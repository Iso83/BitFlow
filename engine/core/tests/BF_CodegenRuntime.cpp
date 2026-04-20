#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/bitvector/BitVector.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
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
    namespace fs = std::filesystem;
    static const fs::path repoRoot = [] {
        fs::path p = fs::path(__FILE__);
        return p.parent_path().parent_path().parent_path().parent_path();
    }();
    const fs::path includeDir = repoRoot / "engine/core/include";
    const fs::path runtimeSrc = repoRoot / "engine/core/src/bitvector/BitVector.cpp";

    const std::string strictFlags = " -Wall -Wextra -Wpedantic -Werror";
    const std::string includeFlags = " -I\"" + includeDir.string() + "\"";
    const std::string runtimeFlags = " \"" + runtimeSrc.string() + "\"";
    int res =
        std::system(("g++ -std=c++20" + strictFlags + includeFlags + " \"" + file + "\"" + runtimeFlags + " -o \"" +
                     exe + "\"")
                        .c_str());
    if (res != 0)
        res = std::system(("c++ -std=c++20" + strictFlags + includeFlags + " \"" + file + "\"" + runtimeFlags +
                           " -o \"" + exe + "\"")
                              .c_str());
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

static bool CompileWrapperOnly(const std::string& wrapper, const std::string& support,
                               const std::string& probeStmt) {
    static std::atomic<uint64_t> counter{200000};
    const uint64_t id = counter.fetch_add(1);

    const std::string base = "bf_codegen_compile_only_" + std::to_string(id);
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
    out << "int main(){\n";
    out << probeStmt << "\n";
    out << "return 0;\n";
    out << "}\n";
    out.close();

    const int res = CompileCppSource(file, exe);
    std::remove(file.c_str());
    if (res == 0)
        std::remove(exe.c_str());
#if defined(_WIN32)
    std::remove((base + ".obj").c_str());
#endif
    return res == 0;
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

    // Wide-bitwidth generated function path (bf_uint runtime contract).
    auto wideVar = MakeVar(50);
    auto wideExpr = MakeOp(51, OpType::RotL, {wideVar, MakeConst(52, 5)});
    auto wideFn = Codegen::EmitCFunction(wideExpr, 128);
    const auto support128 = Codegen::EmitCRuntimeSupport(128);
    auto wideEvalExpr = MakeOp(53, OpType::RotL, {MakeConst(54, 0x81), MakeConst(55, 5)});
    auto wideEval = Eval::EvaluateConstant(wideEvalExpr, 128);
    BF_TEST(wideEval.status == Eval::EvalStatus::Success);
    BF_TEST(CompileAndRunWrapper(wideFn, "f(bf_uint(0x81ull, 128)).ToUint64()", support128) == wideEval.value);

    // 96-bit generated code must compile and run under the same bf_uint contract.
    auto v96 = MakeVar(60);
    auto expr96 = MakeOp(61, OpType::Xor,
                         {MakeOp(62, OpType::RotR, {v96, MakeConst(63, 7)}), MakeConst(64, 0x55)});
    auto fn96 = Codegen::EmitCFunction(expr96, 96);
    const auto support96 = Codegen::EmitCRuntimeSupport(96);
    BF_TEST(CompileWrapperOnly(fn96, support96, "auto out = f(bf_uint(0x123456789ull, 96)).ToUint64(); (void)out;"));

    // 128-bit multi-output codegen should remain compileable with wide runtime support.
    auto wA = MakeVar(70);
    auto wB = MakeVar(71);
    auto wOut1 = MakeOp(72, OpType::Add, {wA, wB});
    auto wOut2 = MakeOp(73, OpType::RotL, {MakeOp(74, OpType::Xor, {wA, wB}), MakeConst(75, 17)});
    std::vector<const AST::Expr*> wideOutputs = {wOut1, wOut2};
    auto wideMultiFn = Codegen::EmitCFunctionMulti(wideOutputs, 128);
    BF_TEST(CompileWrapperOnly(wideMultiFn, support128,
                               "auto r = f(bf_uint(0x1020304050607080ull, 128), "
                               "bf_uint(0x1112131415161718ull, 128)); "
                               "auto out1 = r.out1.ToUint64(); auto out2 = r.out2.ToUint64(); "
                               "(void)out1; (void)out2;"));

    return 0;
}
