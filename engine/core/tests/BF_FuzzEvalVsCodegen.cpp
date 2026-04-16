#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <BitFlow/core/eval/ConstantDetect.h>
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

static uint64_t CompileAndRun(const std::string& expr) {
    static std::atomic<uint64_t> counter{0};
    const uint64_t id = counter.fetch_add(1);

    const std::string base = "bf_codegen_fuzz_" + std::to_string(id);
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

    int res = std::system(("g++ -std=c++20 " + file + " -o " + exe).c_str());
    if (res != 0)
        res = std::system(("c++ -std=c++20 " + file + " -o " + exe).c_str());

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
        return 0;
    }

    char buffer[128] = {0};
    if (!fgets(buffer, sizeof(buffer), pipe)) {
        BF_PCLOSE(pipe);
        std::remove(file.c_str());
        std::remove(exe.c_str());
        return 0;
    }

    BF_PCLOSE(pipe);
    std::remove(file.c_str());
    std::remove(exe.c_str());

    return static_cast<uint64_t>(std::stoull(buffer));
}

static uint32_t seed = 1234567u;

static uint32_t Rnd() {
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return seed;
}

static Expr* GenLeaf() {
    if ((Rnd() % 2u) == 0u)
        return MakeConst(Rnd(), Rnd() & 0xFFu);

    return MakeVar((Rnd() % 10u) + 1u);
}

static Expr* GenExpr(int depth) {
    if (depth <= 0)
        return GenLeaf();

    const uint32_t op = Rnd() % 10u;

    Expr* a = GenExpr(depth - 1);
    Expr* b = GenExpr(depth - 1);

    switch (op) {
    case 0:
        return MakeOp(Rnd(), OpType::Add, {a, b});
    case 1:
        return MakeOp(Rnd(), OpType::Sub, {a, b});
    case 2:
        return MakeOp(Rnd(), OpType::Mul, {a, b});
    case 3:
        return MakeOp(Rnd(), OpType::And, {a, b});
    case 4:
        return MakeOp(Rnd(), OpType::Or, {a, b});
    case 5:
        return MakeOp(Rnd(), OpType::Xor, {a, b});
    case 6:
        return MakeOp(Rnd(), OpType::Shl, {a, b});
    case 7:
        return MakeOp(Rnd(), OpType::Shr, {a, b});
    case 8:
        return MakeOp(Rnd(), OpType::RotL, {a, b});
    case 9:
        return MakeOp(Rnd(), OpType::RotR, {a, b});
    default:
        return a;
    }
}

int TestFuzzEvalVsCodegen_32bit() {
    int executed = 0;
    constexpr int kCases = 400;

    for (int i = 0; i < kCases; ++i) {
        Expr* root = GenExpr(2);

        if (!Eval::IsFullyConstant(root))
            continue;

        auto eval = Eval::EvaluateConstant(root, 32);
        if (eval.status != Eval::EvalStatus::Success)
            continue;

        auto code = Codegen::EmitCExpr(root, 32);
        if (code.find("unsupported") != std::string::npos || code.find("invalid") != std::string::npos)
            continue;

        auto run = CompileAndRun(code);
        BF_TEST(eval.value == run);
        ++executed;
    }

    BF_TEST(executed >= 10);
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestFuzzEvalVsCodegen_32bit);
    return 0;
}
