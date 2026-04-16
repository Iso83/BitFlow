#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <random>
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

AST::Expr* GenConst(std::mt19937_64& rng, uint32_t& nextId) {
    std::uniform_int_distribution<uint32_t> valueDist(0u, 0xFFFFFFFFu);
    return MakeConst(nextId++, valueDist(rng));
}

AST::Expr* GenExpr(std::mt19937_64& rng, uint32_t& nextId, int depth) {
    static constexpr OpType unaryOps[] = {OpType::Not, OpType::Neg};
    static constexpr OpType binaryOps[] = {
        OpType::Add, OpType::Sub, OpType::Mul, OpType::Div, OpType::Mod, OpType::And,
        OpType::Or,  OpType::Xor, OpType::Shl, OpType::Shr, OpType::UShr, OpType::RotL,
        OpType::RotR,
    };

    if (depth <= 0)
        return GenConst(rng, nextId);

    std::uniform_int_distribution<int> kindDist(0, 4);
    const int kind = kindDist(rng);

    if (kind == 0)
        return GenConst(rng, nextId);

    if (kind == 1) {
        std::uniform_int_distribution<size_t> opDist(0, std::size(unaryOps) - 1);
        AST::Expr* child = GenExpr(rng, nextId, depth - 1);
        return MakeOp(nextId++, unaryOps[opDist(rng)], {child});
    }

    std::uniform_int_distribution<size_t> opDist(0, std::size(binaryOps) - 1);
    OpType op = binaryOps[opDist(rng)];
    AST::Expr* a = GenExpr(rng, nextId, depth - 1);
    AST::Expr* b = GenExpr(rng, nextId, depth - 1);

    // Avoid UB in generated C++ for div/mod by zero.
    if ((op == OpType::Div || op == OpType::Mod) && b->op == OpType::Const && b->constValue == 0)
        b->constValue = 1;

    return MakeOp(nextId++, op, {a, b});
}

int TestFuzzEvalVsCodegen_ConstOnly_32bit() {
    std::mt19937_64 rng(0xB17F10ULL);

    int executed = 0;
    constexpr int kCases = 120;

    for (int i = 0; i < kCases; ++i) {
        uint32_t nextId = static_cast<uint32_t>(1000 + i * 64);
        AST::Expr* root = GenExpr(rng, nextId, 4);

        auto eval = Eval::EvaluateConstant(root, 32);
        if (eval.status != Eval::EvalStatus::Success)
            continue;

        std::string code = Codegen::EmitCExpr(root, 32);
        if (code.find("unsupported") != std::string::npos || code.find("invalid") != std::string::npos)
            continue;

        uint64_t runtime = CompileAndRun(code);
        BF_TEST(eval.value == runtime);
        ++executed;
    }

    BF_TEST(executed >= 90);
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestFuzzEvalVsCodegen_ConstOnly_32bit);
    return 0;
}
