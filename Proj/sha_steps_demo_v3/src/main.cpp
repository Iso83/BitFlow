#include "ShaDemoSupport.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#if defined(SHA_DEMO_HAS_CLI11)
#include <CLI/CLI.hpp>
#endif

namespace {

using namespace ShaDemo;

RewriteProfile ResolveProfile(const DemoOptions& opt) {
    if (opt.explore)
        return RewriteProfile::ExploreAggressive;
    if (opt.factorize)
        return RewriteProfile::ShaFactorize;
    if (opt.normalizeOnly)
        return RewriteProfile::NormalizeOnly;
    return RewriteProfile::ShaSafe;
}

DemoOptions ParseArgs(int argc, char** argv) {
    DemoOptions opt{};

#if defined(SHA_DEMO_HAS_CLI11)
    CLI::App app{"SHA symbolic step explorer"};
    app.add_option("--steps", opt.steps, "Number of SHA-256 steps to build")->check(CLI::Range(1, 64));
    app.add_option("--console-max", opt.consoleMax, "Max chars per console expression line");
    app.add_option("--out", opt.outDir, "Output directory");

    app.add_flag("--emit-c", opt.emitC, "Write C emitter output");
    app.add_flag("--ssa", opt.ssa, "Write SSA output");
    app.add_flag("--trace", opt.trace, "Enable verbose rewrite trace");

    app.add_flag("--factorize", opt.factorize, "Enable safe factorize rewrite pass");
    app.add_flag("--explore", opt.explore, "Enable aggressive exploratory factorize/distribute pass");
    app.add_flag("--normalize-only", opt.normalizeOnly, "Only normalize expressions");

    app.add_flag("--sha-rules", opt.shaRules, "Enable SHA-specific simplify rules (can be unstable)");
    app.add_flag("--no-schedule", opt.buildSchedule, "Keep only initial W[0..15] constants")
        ->default_val(true)
        ->disable_flag_override();
    app.add_flag("--no-files", opt.writeFiles, "Disable file output")->default_val(true)->disable_flag_override();
    app.add_flag("--no-verify", opt.verify, "Disable concrete verification")
        ->default_val(true)
        ->disable_flag_override();

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        std::exit(app.exit(e));
    }
#else
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--steps" && i + 1 < argc) {
            opt.steps = static_cast<unsigned>(std::strtoul(argv[++i], nullptr, 10));
            continue;
        }
        if (arg == "--console-max" && i + 1 < argc) {
            opt.consoleMax = static_cast<std::size_t>(std::strtoull(argv[++i], nullptr, 10));
            continue;
        }
        if (arg == "--out" && i + 1 < argc) {
            opt.outDir = argv[++i];
            continue;
        }
        if (arg == "--emit-c") {
            opt.emitC = true;
            continue;
        }
        if (arg == "--ssa") {
            opt.ssa = true;
            continue;
        }
        if (arg == "--trace") {
            opt.trace = true;
            continue;
        }
        if (arg == "--factorize") {
            opt.factorize = true;
            continue;
        }
        if (arg == "--explore") {
            opt.explore = true;
            continue;
        }
        if (arg == "--normalize-only") {
            opt.normalizeOnly = true;
            continue;
        }
        if (arg == "--sha-rules") {
            opt.shaRules = true;
            continue;
        }
        if (arg == "--no-sha-rules") {
            opt.shaRules = false;
            continue;
        }
        if (arg == "--no-schedule") {
            opt.buildSchedule = false;
            continue;
        }
        if (arg == "--no-files") {
            opt.writeFiles = false;
            continue;
        }
        if (arg == "--no-verify") {
            opt.verify = false;
            continue;
        }
        throw std::runtime_error("Unknown argument: " + arg);
    }
#endif

    if (opt.steps == 0 || opt.steps > 64)
        throw std::runtime_error("--steps must be in range 1..64");

    if (!opt.buildSchedule && opt.steps > 16)
        throw std::runtime_error("--no-schedule only supports --steps <= 16");

    if (opt.explore && opt.factorize)
        std::cout << "warning: --explore overrides --factorize.\n";

    return opt;
}

void PrintCompact(const char* name, const Expr* expr, const std::unordered_map<uint32_t, std::string>& names,
                  std::size_t maxLen) {
    const auto text = ExprToString(expr, names);
    std::cout << "    " << name << " = " << Truncate(text, maxLen) << "\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
        DemoOptions opt = ParseArgs(argc, argv);
        const RewriteProfile profile = ResolveProfile(opt);

        if ((profile == RewriteProfile::ShaFactorize || profile == RewriteProfile::ExploreAggressive) && opt.verify) {
            std::cout << "warning: verification auto-disabled for chosen factorize mode because current BitFlow "
                         "factorize rules can still change semantics in this snapshot.\n\n";
            opt.verify = false;
        }

        if (opt.shaRules && opt.verify) {
            std::cout << "warning: verification auto-disabled when --sha-rules is enabled because current SHA rewrite "
                         "rules are unstable in this BitFlow snapshot.\n\n";
            opt.verify = false;
        }

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

        std::cout << "SHA symbolic step explorer\n";
        std::cout << "steps       : " << opt.steps << "\n";
        std::cout << "mode        : " << RewriteProfileName(profile) << "\n";
        std::cout << "sha rules   : " << (opt.shaRules ? "on" : "off") << "\n";
        std::cout << "schedule    : " << (opt.buildSchedule ? "full until step" : "W[0..15] only") << "\n";
        std::cout << "trace       : " << (opt.trace ? "on" : "off") << "\n";
        std::cout << "write files : " << (opt.writeFiles ? "on" : "off") << "\n";
        std::cout << "verify      : " << (opt.verify ? "on" : "off") << "\n";
        std::cout << "\n";

        std::filesystem::path outRoot = opt.outDir;
        if (opt.writeFiles)
            EnsureDir(outRoot);

        for (unsigned step = 0; step < opt.steps; ++step) {
            std::cout << "============================================================\n";
            std::cout << "step " << step << "\n";
            std::cout << "    k = " << Hex32(kSha256[step]) << "\n";

            Expr* wExpr = nullptr;
            if (step < 16 || !opt.buildSchedule) {
                wExpr = scheduleExprs[step];
            } else {
                wExpr = BuildScheduleExprAt(b, scheduleExprs, step, b.Names(), profile, opt.trace, opt.shaRules);
            }

            const auto raw = BuildStep(b, symbolic, wExpr, kSha256[step]);
            const auto t1rw = RewriteExpr(raw.t1, b.Names(), profile, false, opt.shaRules);
            const auto t2rw = RewriteExpr(raw.t2, b.Names(), profile, false, opt.shaRules);
            const auto rewritten = RewriteStepOutputs(raw, b.Names(), profile, opt.trace, opt.shaRules);

            PrintCompact(("w" + std::to_string(step)).c_str(), wExpr, b.Names(), opt.consoleMax);
            PrintCompact("t1", t1rw, b.Names(), opt.consoleMax);
            PrintCompact("t2", t2rw, b.Names(), opt.consoleMax);
            PrintCompact("a_next", rewritten.a, b.Names(), opt.consoleMax);
            PrintCompact("e_next", rewritten.e, b.Names(), opt.consoleMax);

            std::filesystem::path stepDir = outRoot / ("step_" + std::to_string(step));
            if (opt.writeFiles) {
                EnsureDir(stepDir);
                DumpExprBundle(stepDir, "w", wExpr, b.Names(), opt.emitC, opt.ssa);
                DumpExprBundle(stepDir, "t1", t1rw, b.Names(), opt.emitC, opt.ssa);
                DumpExprBundle(stepDir, "t2", t2rw, b.Names(), opt.emitC, opt.ssa);
                DumpExprBundle(stepDir, "a_next", rewritten.a, b.Names(), opt.emitC, opt.ssa);
                DumpExprBundle(stepDir, "e_next", rewritten.e, b.Names(), opt.emitC, opt.ssa);
            }

            if (opt.verify) {
                const StateVal refNext = RefStep(concrete, kSha256[step], scheduleValues[step]);
                const StateVal bfNext =
                    VerifyConcreteStep(concrete, kSha256[step], scheduleValues[step], profile, opt.shaRules);

                std::cout << "    verify a: ref=" << Hex32(refNext.a) << " bitflow=" << Hex32(bfNext.a)
                          << (refNext.a == bfNext.a ? "  OK" : "  FAIL") << "\n";
                std::cout << "    verify e: ref=" << Hex32(refNext.e) << " bitflow=" << Hex32(bfNext.e)
                          << (refNext.e == bfNext.e ? "  OK" : "  FAIL") << "\n";

                if (refNext.a != bfNext.a || refNext.b != bfNext.b || refNext.c != bfNext.c || refNext.d != bfNext.d ||
                    refNext.e != bfNext.e || refNext.f != bfNext.f || refNext.g != bfNext.g || refNext.h != bfNext.h) {
                    throw std::runtime_error("Concrete verification mismatch at step " + std::to_string(step));
                }

                if (opt.writeFiles) {
                    std::ostringstream oss;
                    oss << "verify a: ref=" << Hex32(refNext.a) << " bitflow=" << Hex32(bfNext.a) << "\n";
                    oss << "verify e: ref=" << Hex32(refNext.e) << " bitflow=" << Hex32(bfNext.e) << "\n";
                    WriteTextFile(stepDir / "summary.txt", oss.str());
                }

                concrete = refNext;
            }

            symbolic = rewritten;
        }

        if (opt.verify) {
            std::cout << "\nfinal concrete state after " << opt.steps << " steps\n";
            std::cout << "  a=" << Hex32(concrete.a) << " b=" << Hex32(concrete.b) << " c=" << Hex32(concrete.c)
                      << " d=" << Hex32(concrete.d) << "\n";
            std::cout << "  e=" << Hex32(concrete.e) << " f=" << Hex32(concrete.f) << " g=" << Hex32(concrete.g)
                      << " h=" << Hex32(concrete.h) << "\n";
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 1;
    }
}
