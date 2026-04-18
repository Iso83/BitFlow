#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <atomic>
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

static int CompileCppSource(const std::string& file, const std::string& exe) {
    int res = std::system(("g++ -std=c++20 " + file + " -o " + exe).c_str());
    if (res != 0)
        res = std::system(("c++ -std=c++20 " + file + " -o " + exe).c_str());
#if defined(_WIN32)
    if (res != 0)
        res = std::system(("cl /nologo /std:c++20 /O2 /EHsc " + file + " /Fe:" + exe).c_str());
#endif
    return res;
}

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
    auto c1 = MakeConst(1, 10);
    auto c2 = MakeConst(2, 20);
    auto expr = MakeOp(3, OpType::Add, {c1, c2});
    auto eval = Eval::EvaluateConstant(expr, 32);
    BF_TEST(eval.status == Eval::EvalStatus::Success);
    BF_TEST(eval.value == CompileAndRun(Codegen::EmitCExpr(expr, 32)));

    auto a = MakeVar(11);
    auto b = MakeVar(12);
    auto wrapperExpr = MakeOp(13, OpType::Mul, {MakeOp(14, OpType::Add, {a, b}), MakeConst(15, 2)});
    auto wrapper = Codegen::EmitCFunction(wrapperExpr, 32);
    BF_TEST(CompileAndRunWrapper(wrapper, "f(5, 7)") == 24ull);

    return 0;
}
