#pragma once

#include "ShaExprBuilder.h"

#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <BitFlow/io/ExprPrinter.h>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace SHA_Demo {

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
    StateExpr next;
};

inline std::string Hex32(uint32_t v) {
    std::ostringstream oss;
    oss << "0x" << std::hex << std::setfill('0') << std::setw(8) << v;
    return oss.str();
}

inline std::string Shorten(std::string s, std::size_t maxLen) {
    if (s.size() <= maxLen) {
        return s;
    }
    return s.substr(0, maxLen) + " ... [truncated, len=" + std::to_string(s.size()) + "]";
}

inline void WriteTextFile(const std::filesystem::path& path, const std::string& text) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Cannot open output file: " + path.string());
    }
    out << text;
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

    for (size_t i = 0; i < 16; ++i) {
        w[i] = first[i];
    }

    for (unsigned i = 16; i < needed; ++i) {
        w[i] = SmallSigma1Ref(w[i - 2]) + w[i - 7] + SmallSigma0Ref(w[i - 15]) + w[i - 16];
    }

    return w;
}

inline Expr* RewriteExpr(Expr* root, bool enableShaRules) {
    using namespace BitFlow::Core::Rules;

    {
        RuleEngine engine;
        Add_Normalize_Rules(engine);
        root = engine.ApplyUntilStable(root);
    }
    {
        RuleEngine engine;
        Add_Normalize_Rules(engine);
        Add_Simplify_Bitwise_Rules(engine);
        root = engine.ApplyUntilStable(root);
    }
    {
        RuleEngine engine;
        Add_Normalize_Rules(engine);
        Add_Simplify_Arithmetic_Rules(engine);
        root = engine.ApplyUntilStable(root);
    }

    if (enableShaRules) {
        RuleEngine engine;
        Add_Normalize_Rules(engine);
        Add_Simplify_Bitwise_Rules(engine);
        Add_Simplify_Arithmetic_Rules(engine);
        Add_Simplify_SHA_Rules(engine);
        root = engine.ApplyUntilStable(root);
    }

    return root;
}

inline uint32_t EvalExprConstant(const Expr* expr) {
    const auto result = BitFlow::Core::Eval::EvaluateConstant(expr, 32);
    if (result.status != BitFlow::Core::Eval::EvalStatus::Success) {
        throw std::runtime_error("EvaluateConstant failed for concrete expression.");
    }
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

inline StateExpr RewriteStepOutputs(const StepArtifacts& art, bool enableShaRules) {
    StateExpr out{};
    out.a = RewriteExpr(art.next.a, enableShaRules);
    out.b = RewriteExpr(art.next.b, enableShaRules);
    out.c = RewriteExpr(art.next.c, enableShaRules);
    out.d = RewriteExpr(art.next.d, enableShaRules);
    out.e = RewriteExpr(art.next.e, enableShaRules);
    out.f = RewriteExpr(art.next.f, enableShaRules);
    out.g = RewriteExpr(art.next.g, enableShaRules);
    out.h = RewriteExpr(art.next.h, enableShaRules);
    return out;
}

inline Expr* BuildScheduleExprAt(ExprBuilder& b, std::vector<Expr*>& schedule, unsigned index, bool enableShaRules) {
    while (schedule.size() <= index) {
        const size_t i = schedule.size();
        Expr* expr =
            b.Add({b.SmallSigma1(schedule[i - 2]), schedule[i - 7], b.SmallSigma0(schedule[i - 15]), schedule[i - 16]});

        expr = RewriteExpr(expr, enableShaRules);
        schedule.push_back(expr);
    }

    return schedule[index];
}

inline StateVal VerifyConcreteStep(const StateVal& current, uint32_t k, uint32_t wValue, bool enableShaRules) {
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
    const auto rewritten = RewriteStepOutputs(concrete, enableShaRules);

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

inline std::string ExprString(const Expr* expr, const std::unordered_map<uint32_t, std::string>& names) {
    return BitFlow::IO::ToString(expr, names);
}

inline std::string EmitCString(const Expr* expr) {
    return BitFlow::Core::Codegen::EmitCExpr(expr, 32);
}

inline void DumpExprFile(const std::filesystem::path& dir, const std::string& fileName, const Expr* expr,
                         const std::unordered_map<uint32_t, std::string>& names) {
    std::ostringstream oss;
    const std::string text = ExprString(expr, names);
    oss << text << "\\n\\n";
    oss << "[c]\\n";
    oss << EmitCString(expr) << "\\n";
    WriteTextFile(dir / fileName, oss.str());
}

inline void DumpStepSummaryFile(const std::filesystem::path& dir, unsigned step, uint32_t k, uint32_t w,
                                const StateVal& refNext, const StateVal& bitflowNext) {
    std::ostringstream oss;
    oss << "step=" << step << "\\n";
    oss << "k=" << Hex32(k) << "\\n";
    oss << "w=" << Hex32(w) << "\\n\\n";

    oss << "ref_next\\n";
    oss << "a=" << Hex32(refNext.a) << "\\n";
    oss << "b=" << Hex32(refNext.b) << "\\n";
    oss << "c=" << Hex32(refNext.c) << "\\n";
    oss << "d=" << Hex32(refNext.d) << "\\n";
    oss << "e=" << Hex32(refNext.e) << "\\n";
    oss << "f=" << Hex32(refNext.f) << "\\n";
    oss << "g=" << Hex32(refNext.g) << "\\n";
    oss << "h=" << Hex32(refNext.h) << "\\n\\n";

    oss << "bitflow_next\\n";
    oss << "a=" << Hex32(bitflowNext.a) << "\\n";
    oss << "b=" << Hex32(bitflowNext.b) << "\\n";
    oss << "c=" << Hex32(bitflowNext.c) << "\\n";
    oss << "d=" << Hex32(bitflowNext.d) << "\\n";
    oss << "e=" << Hex32(bitflowNext.e) << "\\n";
    oss << "f=" << Hex32(bitflowNext.f) << "\\n";
    oss << "g=" << Hex32(bitflowNext.g) << "\\n";
    oss << "h=" << Hex32(bitflowNext.h) << "\\n";
    WriteTextFile(dir / "summary.txt", oss.str());
}

} // namespace SHA_Demo
