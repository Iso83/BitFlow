#include "ShaExprBuilder.h"

#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <BitFlow/io/ExprPrinter.h>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using BitFlow::Core::AST::Expr;
using DemoSHA::ExprBuilder;

constexpr std::array<uint32_t, 64> kSha256 = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u,
};

struct StateExpr {
    Expr* a = nullptr;
    Expr* b = nullptr;
    Expr* c = nullptr;
    Expr* d = nullptr;
    Expr* e = nullptr;
    Expr* f = nullptr;
    Expr* g = nullptr;
    Expr* h = nullptr;
};

struct StateVal {
    uint32_t a = 0;
    uint32_t b = 0;
    uint32_t c = 0;
    uint32_t d = 0;
    uint32_t e = 0;
    uint32_t f = 0;
    uint32_t g = 0;
    uint32_t h = 0;
};

struct CliOptions {
    unsigned steps = 4;
    bool factorize = false;
    bool ssa = false;
    bool emitC = false;
    bool verboseTrace = true;
};

struct StepArtifacts {
    Expr* w = nullptr;
    Expr* t1 = nullptr;
    Expr* t2 = nullptr;
    StateExpr next;
};

std::string Hex32(uint32_t v) {
    std::ostringstream oss;
    oss << "0x" << std::hex << std::setfill('0') << std::setw(8) << v;
    return oss.str();
}

uint32_t RotR32(uint32_t x, uint32_t n) {
    n &= 31u;
    return (x >> n) | (x << ((32u - n) & 31u));
}

uint32_t BigSigma0Ref(uint32_t x) {
    return RotR32(x, 2) ^ RotR32(x, 13) ^ RotR32(x, 22);
}

uint32_t BigSigma1Ref(uint32_t x) {
    return RotR32(x, 6) ^ RotR32(x, 11) ^ RotR32(x, 25);
}

uint32_t SmallSigma0Ref(uint32_t x) {
    return RotR32(x, 7) ^ RotR32(x, 18) ^ (x >> 3);
}

uint32_t SmallSigma1Ref(uint32_t x) {
    return RotR32(x, 17) ^ RotR32(x, 19) ^ (x >> 10);
}

uint32_t ChRef(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

uint32_t MajRef(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

std::array<uint32_t, 16> LoadBlockWords(const std::array<uint8_t, 64>& block) {
    std::array<uint32_t, 16> words{};
    for (size_t i = 0; i < 16; ++i) {
        const size_t j = i * 4;
        words[i] = (uint32_t(block[j]) << 24) | (uint32_t(block[j + 1]) << 16) | (uint32_t(block[j + 2]) << 8) |
                   uint32_t(block[j + 3]);
    }
    return words;
}

std::vector<uint32_t> BuildScheduleReference(const std::array<uint8_t, 64>& block, unsigned steps) {
    const unsigned needed = (steps > 16u) ? steps : 16u;
    std::vector<uint32_t> w(needed, 0u);
    const auto first = LoadBlockWords(block);
    for (size_t i = 0; i < 16; ++i)
        w[i] = first[i];
    for (unsigned i = 16; i < needed; ++i)
        w[i] = SmallSigma1Ref(w[i - 2]) + w[i - 7] + SmallSigma0Ref(w[i - 15]) + w[i - 16];
    return w;
}

Expr* RewriteExpr(Expr* root, const std::unordered_map<uint32_t, std::string>& names, const std::string& label,
                  bool factorize, bool verboseTrace) {
    using namespace BitFlow::Core::Rules;

    const auto setCallback = [&](RuleEngine& engine, const char* stageName) {
        engine.SetDebugCallback([&](const Expr* before, const Expr* after, RuleId id) {
            if (!verboseTrace)
                return;
            std::cout << "      [" << stageName << "] rule=" << static_cast<int>(id) << "\n";
            std::cout << "        before: " << BitFlow::IO::ToString(before, names) << "\n";
            std::cout << "        after : " << BitFlow::IO::ToString(after, names) << "\n";
        });
    };

    if (verboseTrace)
        std::cout << "    rewrite root: " << label << "\n";

    {
        RuleEngine engine;
        Add_Normalize_Rules(engine);
        setCallback(engine, "normalize");
        root = engine.ApplyUntilStable(root);
    }
    {
        RuleEngine engine;
        Add_Normalize_Rules(engine);
        Add_Simplify_Bitwise_Rules(engine);
        setCallback(engine, "simplify-bitwise");
        root = engine.ApplyUntilStable(root);
    }
    {
        RuleEngine engine;
        Add_Normalize_Rules(engine);
        Add_Simplify_Arithmetic_Rules(engine);
        setCallback(engine, "simplify-arithmetic");
        root = engine.ApplyUntilStable(root);
    }
    {
        RuleEngine engine;
        Add_Normalize_Rules(engine);
        Add_Simplify_Bitwise_Rules(engine);
        Add_Simplify_Arithmetic_Rules(engine);
        Add_Simplify_SHA_Rules(engine);
        setCallback(engine, "simplify-sha");
        root = engine.ApplyUntilStable(root);
    }

    if (factorize) {
        RuleEngine bitwiseEngine;
        Add_Normalize_Rules(bitwiseEngine);
        Add_Simplify_Bitwise_Rules(bitwiseEngine);
        Add_Factorize_Bitwise_Rules(bitwiseEngine);
        setCallback(bitwiseEngine, "factorize-bitwise");
        root = bitwiseEngine.ApplyUntilStable(root);

        RuleEngine arithmeticEngine;
        Add_Normalize_Rules(arithmeticEngine);
        Add_Simplify_Arithmetic_Rules(arithmeticEngine);
        Add_Factorize_Arithmetic_Rules(arithmeticEngine);
        setCallback(arithmeticEngine, "factorize-arithmetic");
        root = arithmeticEngine.ApplyUntilStable(root);
    }

    return root;
}

void PrintExprSummary(const char* name, const Expr* expr, const std::unordered_map<uint32_t, std::string>& names,
                      bool ssa, bool emitC) {
    std::cout << "    " << name << " = " << BitFlow::IO::ToString(expr, names) << "\n";

    if (ssa) {
        const auto prog = BitFlow::Core::Codegen::BuildSSA(expr, 32);
        std::cout << "    " << name << " [ssa]" << "\n";
        for (const auto& stmt : prog.statements)
            std::cout << "      " << stmt.name << " = " << stmt.expr << "\n";
        std::cout << "      result = " << prog.result << "\n";
    }

    if (emitC)
        std::cout << "    " << name << " [c] = " << BitFlow::Core::Codegen::EmitCExpr(expr, 32) << "\n";
}

uint32_t EvalExprConstant(const Expr* expr) {
    const auto result = BitFlow::Core::Eval::EvaluateConstant(expr, 32);
    if (result.status != BitFlow::Core::Eval::EvalStatus::Success)
        throw std::runtime_error("EvaluateConstant failed for concrete expression.");
    return static_cast<uint32_t>(result.value);
}

StateVal RefStep(const StateVal& s, uint32_t k, uint32_t w) {
    const uint32_t t1 = s.h + BigSigma1Ref(s.e) + ChRef(s.e, s.f, s.g) + k + w;
    const uint32_t t2 = BigSigma0Ref(s.a) + MajRef(s.a, s.b, s.c);
    StateVal out{};
    out.a = t1 + t2;
    out.b = s.a;
    out.c = s.b;
    out.d = s.c;
    out.e = s.d + t1;
    out.f = s.e;
    out.g = s.f;
    out.h = s.g;
    return out;
}

StepArtifacts BuildStep(ExprBuilder& b, const StateExpr& s, Expr* w, uint32_t k) {
    StepArtifacts art{};
    art.w = w;
    art.t1 = b.Add({s.h, b.BigSigma1(s.e), b.Ch(s.e, s.f, s.g), b.Const(k), w});
    art.t2 = b.Add({b.BigSigma0(s.a), b.Maj(s.a, s.b, s.c)});
    art.next.a = b.Add({art.t1, art.t2});
    art.next.b = s.a;
    art.next.c = s.b;
    art.next.d = s.c;
    art.next.e = b.Add({s.d, art.t1});
    art.next.f = s.e;
    art.next.g = s.f;
    art.next.h = s.g;
    return art;
}

StateExpr RewriteStepOutputs(const StepArtifacts& art, const std::unordered_map<uint32_t, std::string>& names,
                             bool factorize, bool verboseTrace) {
    StateExpr out{};
    out.a = RewriteExpr(art.next.a, names, "a_next", factorize, verboseTrace);
    out.b = RewriteExpr(art.next.b, names, "b_next", factorize, verboseTrace);
    out.c = RewriteExpr(art.next.c, names, "c_next", factorize, verboseTrace);
    out.d = RewriteExpr(art.next.d, names, "d_next", factorize, verboseTrace);
    out.e = RewriteExpr(art.next.e, names, "e_next", factorize, verboseTrace);
    out.f = RewriteExpr(art.next.f, names, "f_next", factorize, verboseTrace);
    out.g = RewriteExpr(art.next.g, names, "g_next", factorize, verboseTrace);
    out.h = RewriteExpr(art.next.h, names, "h_next", factorize, verboseTrace);
    return out;
}

Expr* BuildScheduleExprAt(ExprBuilder& b, std::vector<Expr*>& schedule, unsigned index,
                          const std::unordered_map<uint32_t, std::string>& names, bool factorize, bool verboseTrace) {
    while (schedule.size() <= index) {
        const size_t i = schedule.size();
        Expr* expr =
            b.Add({b.SmallSigma1(schedule[i - 2]), schedule[i - 7], b.SmallSigma0(schedule[i - 15]), schedule[i - 16]});
        expr = RewriteExpr(expr, names, "w" + std::to_string(i), factorize, verboseTrace);
        schedule.push_back(expr);
    }
    return schedule[index];
}

StateVal VerifyConcreteStep(const StateVal& current, uint32_t k, uint32_t wValue, bool factorize, bool verboseTrace) {
    ExprBuilder b(500000);
    StateExpr c{};
    c.a = b.Const(current.a);
    c.b = b.Const(current.b);
    c.c = b.Const(current.c);
    c.d = b.Const(current.d);
    c.e = b.Const(current.e);
    c.f = b.Const(current.f);
    c.g = b.Const(current.g);
    c.h = b.Const(current.h);

    auto concrete = BuildStep(b, c, b.Const(wValue), k);
    const auto rewritten = RewriteStepOutputs(concrete, b.Names(), factorize, verboseTrace);

    StateVal out{};
    out.a = EvalExprConstant(rewritten.a);
    out.b = EvalExprConstant(rewritten.b);
    out.c = EvalExprConstant(rewritten.c);
    out.d = EvalExprConstant(rewritten.d);
    out.e = EvalExprConstant(rewritten.e);
    out.f = EvalExprConstant(rewritten.f);
    out.g = EvalExprConstant(rewritten.g);
    out.h = EvalExprConstant(rewritten.h);
    return out;
}

CliOptions ParseArgs(int argc, char** argv) {
    CliOptions opt{};
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--steps" && i + 1 < argc) {
            opt.steps = static_cast<unsigned>(std::strtoul(argv[++i], nullptr, 10));
            continue;
        }
        if (arg == "--no-factorize") {
            opt.factorize = false;
            continue;
        }
        if (arg == "--no-ssa") {
            opt.ssa = false;
            continue;
        }
        if (arg == "--emit-c") {
            opt.emitC = true;
            continue;
        }
        if (arg == "--quiet-trace") {
            opt.verboseTrace = false;
            continue;
        }
        throw std::runtime_error("Unknown argument: " + arg);
    }
    if (opt.steps == 0 || opt.steps > 64)
        throw std::runtime_error("--steps must be in range 1..64");
    return opt;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const CliOptions opt = ParseArgs(argc, argv);

        ExprBuilder b;
        StateExpr symbolic{};
        symbolic.a = b.Var("a0");
        symbolic.b = b.Var("b0");
        symbolic.c = b.Var("c0");
        symbolic.d = b.Var("d0");
        symbolic.e = b.Var("e0");
        symbolic.f = b.Var("f0");
        symbolic.g = b.Var("g0");
        symbolic.h = b.Var("h0");

        StateVal concrete{
            0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au, 0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u,
        };

        const std::array<uint8_t, 64> block = {
            'a', 'b', 'c', 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0,   0,   0,   0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24,
        };

        auto scheduleValues = BuildScheduleReference(block, opt.steps);
        std::vector<Expr*> scheduleExprs;
        scheduleExprs.reserve(opt.steps);
        for (unsigned i = 0; i < 16; ++i)
            scheduleExprs.push_back(b.Const(scheduleValues[i]));

        std::cout << "SHA-256 symbolic step explorer\n";
        std::cout << "steps      : " << opt.steps << "\n";
        std::cout << "factorize  : " << (opt.factorize ? "on" : "off") << "\n";
        std::cout << "ssa        : " << (opt.ssa ? "on" : "off") << "\n";
        std::cout << "emit-c     : " << (opt.emitC ? "on" : "off") << "\n";
        std::cout << "trace      : " << (opt.verboseTrace ? "verbose" : "compact") << "\n\n";

        for (unsigned step = 0; step < opt.steps; ++step) {
            std::cout << "============================================================\n";
            std::cout << "step " << step << "\n";
            std::cout << "k = " << Hex32(kSha256[step]) << "\n";

            Expr* wExpr = nullptr;
            if (step < scheduleExprs.size()) {
                wExpr = scheduleExprs[step];
            } else {
                wExpr = BuildScheduleExprAt(b, scheduleExprs, step, b.Names(), opt.factorize, opt.verboseTrace);
            }

            std::cout << "w = " << Hex32(scheduleValues[step]) << "\n";
            PrintExprSummary(("w" + std::to_string(step)).c_str(), wExpr, b.Names(), opt.ssa, opt.emitC);

            const auto raw = BuildStep(b, symbolic, wExpr, kSha256[step]);
            const auto rewritten = RewriteStepOutputs(raw, b.Names(), opt.factorize, opt.verboseTrace);

            PrintExprSummary("t1", RewriteExpr(raw.t1, b.Names(), "t1", opt.factorize, false), b.Names(), opt.ssa,
                             opt.emitC);
            PrintExprSummary("t2", RewriteExpr(raw.t2, b.Names(), "t2", opt.factorize, false), b.Names(), opt.ssa,
                             opt.emitC);
            PrintExprSummary("a_next", rewritten.a, b.Names(), opt.ssa, opt.emitC);
            PrintExprSummary("e_next", rewritten.e, b.Names(), opt.ssa, opt.emitC);

            const StateVal refNext = RefStep(concrete, kSha256[step], scheduleValues[step]);
            const StateVal bfNext =
                VerifyConcreteStep(concrete, kSha256[step], scheduleValues[step], opt.factorize, false);

            std::cout << "    verify a: ref=" << Hex32(refNext.a) << " bitflow=" << Hex32(bfNext.a)
                      << (refNext.a == bfNext.a ? "  OK" : "  FAIL") << "\n";
            std::cout << "    verify e: ref=" << Hex32(refNext.e) << " bitflow=" << Hex32(bfNext.e)
                      << (refNext.e == bfNext.e ? "  OK" : "  FAIL") << "\n";

            if (refNext.a != bfNext.a || refNext.b != bfNext.b || refNext.c != bfNext.c || refNext.d != bfNext.d ||
                refNext.e != bfNext.e || refNext.f != bfNext.f || refNext.g != bfNext.g || refNext.h != bfNext.h) {
                std::cerr << "Concrete verification mismatch at step " << step << "\n";
                return 2;
            }

            symbolic = rewritten;
            concrete = refNext;
        }

        std::cout << "\nfinal concrete state after " << opt.steps << " steps\n";
        std::cout << "  a=" << Hex32(concrete.a) << " b=" << Hex32(concrete.b) << " c=" << Hex32(concrete.c)
                  << " d=" << Hex32(concrete.d) << "\n";
        std::cout << "  e=" << Hex32(concrete.e) << " f=" << Hex32(concrete.f) << " g=" << Hex32(concrete.g)
                  << " h=" << Hex32(concrete.h) << "\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 1;
    }
}
