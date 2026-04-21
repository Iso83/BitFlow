#pragma once

#include "ShaExprBuilder.h"

#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <BitFlow/io/ExprPrinter.h>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace ShaDemo {

using BitFlow::Core::AST::Expr;

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

struct StepArtifacts {
    Expr* w = nullptr;
    Expr* t1 = nullptr;
    Expr* t2 = nullptr;
    StateExpr next{};
};

struct DemoOptions {
    unsigned steps = 4;
    bool shaRules = true;
    bool factorize = false;
    bool explore = false;
    bool normalizeOnly = false;
    bool buildSchedule = true;
    bool emitC = false;
    bool ssa = false;
    bool writeFiles = true;
    bool verify = true;
    bool trace = false;
    std::size_t consoleMax = 240;
    std::string outDir = "sha_out";
};

enum class RewriteProfile {
    NormalizeOnly,
    ShaSafe,
    ShaFactorize,
    ExploreAggressive,
};

inline std::string Hex32(uint32_t v) {
    std::ostringstream oss;
    oss << "0x" << std::hex << std::setfill('0') << std::setw(8) << v;
    return oss.str();
}

inline uint32_t RotR32(uint32_t x, uint32_t n) {
    n &= 31u;
    return (x >> n) | (x << ((32u - n) & 31u));
}

inline uint32_t BigSigma0Ref(uint32_t x) {
    return RotR32(x, 2) ^ RotR32(x, 13) ^ RotR32(x, 22);
}

inline uint32_t BigSigma1Ref(uint32_t x) {
    return RotR32(x, 6) ^ RotR32(x, 11) ^ RotR32(x, 25);
}

inline uint32_t SmallSigma0Ref(uint32_t x) {
    return RotR32(x, 7) ^ RotR32(x, 18) ^ (x >> 3);
}

inline uint32_t SmallSigma1Ref(uint32_t x) {
    return RotR32(x, 17) ^ RotR32(x, 19) ^ (x >> 10);
}

inline uint32_t ChRef(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

inline uint32_t MajRef(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

inline std::array<uint32_t, 16> LoadBlockWords(const std::array<uint8_t, 64>& block) {
    std::array<uint32_t, 16> words{};
    for (size_t i = 0; i < 16; ++i) {
        const size_t j = i * 4;
        words[i] = (uint32_t(block[j]) << 24) | (uint32_t(block[j + 1]) << 16) | (uint32_t(block[j + 2]) << 8) |
                   uint32_t(block[j + 3]);
    }
    return words;
}

inline std::vector<uint32_t> BuildScheduleReference(const std::array<uint8_t, 64>& block, unsigned steps) {
    const unsigned needed = (steps > 16u) ? steps : 16u;

    std::vector<uint32_t> w(needed, 0u);
    const auto first = LoadBlockWords(block);

    for (size_t i = 0; i < 16; ++i)
        w[i] = first[i];

    for (unsigned i = 16; i < needed; ++i)
        w[i] = SmallSigma1Ref(w[i - 2]) + w[i - 7] + SmallSigma0Ref(w[i - 15]) + w[i - 16];

    return w;
}

inline const char* RewriteProfileName(RewriteProfile profile) {
    switch (profile) {
    case RewriteProfile::NormalizeOnly:
        return "normalize";
    case RewriteProfile::ShaSafe:
        return "sha_safe";
    case RewriteProfile::ShaFactorize:
        return "sha_factorize";
    case RewriteProfile::ExploreAggressive:
        return "explore";
    }

    return "unknown";
}

inline std::string Truncate(const std::string& s, std::size_t maxLen) {
    if (s.size() <= maxLen)
        return s;
    if (maxLen <= 3)
        return s.substr(0, maxLen);
    return s.substr(0, maxLen - 3) + "...";
}

inline void EnsureDir(const std::filesystem::path& path) {
    std::filesystem::create_directories(path);
}

inline void WriteTextFile(const std::filesystem::path& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary);
    out << text;
}

inline std::string ExprToString(const Expr* expr, const std::unordered_map<uint32_t, std::string>& names) {
    return BitFlow::IO::ToString(expr, names);
}

inline std::string ExprToC(const Expr* expr) {
    return BitFlow::Core::Codegen::EmitCExpr(expr, 32);
}

inline std::string ExprToSSA(const Expr* expr) {
    const auto prog = BitFlow::Core::Codegen::BuildSSA(expr, 32);
    std::ostringstream oss;
    for (const auto& stmt : prog.statements)
        oss << stmt.name << " = " << stmt.expr << "\n";
    oss << "result = " << prog.result << "\n";
    return oss.str();
}

inline void AttachTrace(BitFlow::Core::Rules::RuleEngine& engine,
                        const std::unordered_map<uint32_t, std::string>& names, const std::string& stage,
                        bool enabled) {
    if (!enabled)
        return;

    engine.SetDebugCallback([&names, stage](const Expr* before, const Expr* after, BitFlow::Core::Rules::RuleId id) {
        std::cout << "      [" << stage << "] rule=" << static_cast<int>(id) << "\n";
        std::cout << "        before: " << ExprToString(before, names) << "\n";
        std::cout << "        after : " << ExprToString(after, names) << "\n";
    });
}

inline Expr* RunNormalize(Expr* root, const std::unordered_map<uint32_t, std::string>& names, bool trace) {
    BitFlow::Core::Rules::RuleEngine engine;
    BitFlow::Core::Rules::Add_Normalize_Rules(engine);
    AttachTrace(engine, names, "normalize", trace);
    return engine.ApplyUntilStable(root);
}

inline void AddArithmeticSimplifyWithShifts(BitFlow::Core::Rules::RuleEngine& engine) {
    using namespace BitFlow::Core::Rules;
    engine.AddRule(Simplify::Arithmetic::Get_Add_Zero_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Sub_Zero_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Mul_Zero_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Mod_Zero_Guard_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Shift_Zero_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Rotate_Modulo_Bitwidth_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Mul_One_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Div_One_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Neg_Neg_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Add_Fold_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Const_Combine_Rule());
}

inline Expr* RunSimplifySafe(Expr* root, const std::unordered_map<uint32_t, std::string>& names, bool trace) {
    using namespace BitFlow::Core::Rules;

    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Bitwise_Rules(engine);
    AddArithmeticSimplifyWithShifts(engine);
    Add_Simplify_SHA_Rules(engine);

    AttachTrace(engine, names, "sha_safe", trace);
    return engine.ApplyUntilStable(root);
}

inline Expr* RunFactorizeSafe(Expr* root, const std::unordered_map<uint32_t, std::string>& names, bool trace) {
    using namespace BitFlow::Core::Rules;

    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Bitwise_Rules(engine);
    AddArithmeticSimplifyWithShifts(engine);
    Add_Simplify_SHA_Rules(engine);
    engine.AddRule(Factorize::Bitwise::Get_Xor_And_Rule());
    engine.AddRule(Factorize::Bitwise::Get_Xor_Pair_Cancel_Rule());
    engine.AddRule(Factorize::Arithmetic::Get_Add_CommonFactor_Rule());
    engine.AddRule(Factorize::Arithmetic::Get_Mul_CombineConstants_Rule());
    engine.AddRule(Factorize::Bitwise::Get_And_Absorb_Rule());
    engine.AddRule(Factorize::Bitwise::Get_Or_Absorb_Rule());

    AttachTrace(engine, names, "factorize_safe", trace);
    return engine.ApplyUntilStable(root);
}

inline Expr* RunExploreAggressive(Expr* root, const std::unordered_map<uint32_t, std::string>& names, bool trace) {
    using namespace BitFlow::Core::Rules;

    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Bitwise_Rules(engine);
    AddArithmeticSimplifyWithShifts(engine);
    Add_Simplify_SHA_Rules(engine);
    Add_Factorize_Bitwise_Rules(engine);
    Add_Factorize_Arithmetic_Rules(engine);

    AttachTrace(engine, names, "explore", trace);
    return engine.ApplyUntilStable(root);
}

inline Expr* RewriteExpr(Expr* root, const std::unordered_map<uint32_t, std::string>& names, RewriteProfile profile,
                         bool trace) {
    switch (profile) {
    case RewriteProfile::NormalizeOnly:
        return RunNormalize(root, names, trace);
    case RewriteProfile::ShaSafe:
        root = RunNormalize(root, names, trace);
        return RunSimplifySafe(root, names, trace);
    case RewriteProfile::ShaFactorize:
        root = RunNormalize(root, names, trace);
        root = RunSimplifySafe(root, names, trace);
        return RunFactorizeSafe(root, names, trace);
    case RewriteProfile::ExploreAggressive:
        return RunExploreAggressive(root, names, trace);
    }

    return root;
}

inline uint32_t EvalExprConstant(const Expr* expr) {
    const auto result = BitFlow::Core::Eval::EvaluateConstant(expr, 32);
    if (result.status != BitFlow::Core::Eval::EvalStatus::Success)
        throw std::runtime_error("EvaluateConstant failed for concrete expression.");
    return static_cast<uint32_t>(result.value);
}

inline StateVal RefStep(const StateVal& s, uint32_t k, uint32_t w) {
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

inline StepArtifacts BuildStep(ExprBuilder& b, const StateExpr& s, Expr* w, uint32_t k) {
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

inline StateExpr RewriteStepOutputs(const StepArtifacts& art, const std::unordered_map<uint32_t, std::string>& names,
                                    RewriteProfile profile, bool trace) {
    StateExpr out{};
    out.a = RewriteExpr(art.next.a, names, profile, trace);
    out.b = RewriteExpr(art.next.b, names, profile, trace);
    out.c = RewriteExpr(art.next.c, names, profile, trace);
    out.d = RewriteExpr(art.next.d, names, profile, trace);
    out.e = RewriteExpr(art.next.e, names, profile, trace);
    out.f = RewriteExpr(art.next.f, names, profile, trace);
    out.g = RewriteExpr(art.next.g, names, profile, trace);
    out.h = RewriteExpr(art.next.h, names, profile, trace);
    return out;
}

inline Expr* BuildScheduleExprAt(ExprBuilder& b, std::vector<Expr*>& schedule, unsigned index,
                                 const std::unordered_map<uint32_t, std::string>& names, RewriteProfile profile,
                                 bool trace) {
    while (schedule.size() <= index) {
        const size_t i = schedule.size();
        Expr* expr =
            b.Add({b.SmallSigma1(schedule[i - 2]), schedule[i - 7], b.SmallSigma0(schedule[i - 15]), schedule[i - 16]});

        expr = RewriteExpr(expr, names, profile, trace);
        schedule.push_back(expr);
    }

    return schedule[index];
}

inline StateVal VerifyConcreteStep(const StateVal& current, uint32_t k, uint32_t wValue, RewriteProfile profile) {
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
    const auto rewritten = RewriteStepOutputs(concrete, b.Names(), profile, false);

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

inline void DumpExprBundle(const std::filesystem::path& dir, const std::string& baseName, const Expr* expr,
                           const std::unordered_map<uint32_t, std::string>& names, bool emitC, bool ssa) {
    WriteTextFile(dir / (baseName + ".txt"), ExprToString(expr, names) + "\n");
    if (emitC)
        WriteTextFile(dir / (baseName + ".c.txt"), ExprToC(expr) + "\n");
    if (ssa)
        WriteTextFile(dir / (baseName + ".ssa.txt"), ExprToSSA(expr));
}

} // namespace ShaDemo
