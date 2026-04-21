#include "ShaDemoSupport.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#if __has_include(<CLI/CLI.hpp>)
#include <CLI/CLI.hpp>
#define SHA_DEMO_HAS_CLI11 1
#else
#define SHA_DEMO_HAS_CLI11 0
#endif

namespace {

struct Options {
    unsigned steps = 2;
    bool shaRules = true;
    bool buildSchedule = true;
    bool dumpFiles = true;
    std::size_t consoleMax = 420;
    std::filesystem::path outDir = "sha_steps_out";
};

void ParseArgs(int argc, char** argv, Options& opt) {
#if SHA_DEMO_HAS_CLI11
    CLI::App app{"SHA step explorer based on BitFlow"};

    app.add_option("--steps", opt.steps, "Number of rounds to build")->check(CLI::Range(1, 64));
    app.add_flag("--no-sha-rules", opt.shaRules, "Disable SHA-specific simplify rules")
        ->default_val(true)
        ->disable_flag_override();
    app.add_flag("--no-schedule", opt.buildSchedule, "Keep only initial W[0..15] constants")
        ->default_val(true)
        ->disable_flag_override();
    app.add_flag("--no-dump", opt.dumpFiles, "Do not write output files")->default_val(true)->disable_flag_override();
    app.add_option("--console-max", opt.consoleMax, "Max visible chars per console expression");
    app.add_option("--out", opt.outDir, "Output directory");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        std::exit(app.exit(e));
    }
#else
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--steps" && i + 1 < argc) {
            opt.steps = static_cast<unsigned>(std::stoul(argv[++i]));
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
        if (arg == "--no-dump") {
            opt.dumpFiles = false;
            continue;
        }
        if (arg == "--console-max" && i + 1 < argc) {
            opt.consoleMax = static_cast<std::size_t>(std::stoull(argv[++i]));
            continue;
        }
        if (arg == "--out" && i + 1 < argc) {
            opt.outDir = argv[++i];
            continue;
        }
        throw std::runtime_error("Unknown argument: " + arg);
    }
#endif

    if (opt.steps == 0 || opt.steps > 64) {
        throw std::runtime_error("--steps must be 1..64");
    }
}

void PrintExprLine(const std::string& name, const BitFlow::Core::AST::Expr* expr,
                   const std::unordered_map<uint32_t, std::string>& names, std::size_t consoleMax) {
    const std::string text = SHA_Demo::ExprString(expr, names);
    std::cout << "    " << name << " = " << SHA_Demo::Shorten(text, consoleMax) << "\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
        Options opt;
        ParseArgs(argc, argv, opt);

        SHA_Demo::ExprBuilder b;

        SHA_Demo::StateExpr symbolic{};
        symbolic.a = b.Var("a0");
        symbolic.b = b.Var("b0");
        symbolic.c = b.Var("c0");
        symbolic.d = b.Var("d0");
        symbolic.e = b.Var("e0");
        symbolic.f = b.Var("f0");
        symbolic.g = b.Var("g0");
        symbolic.h = b.Var("h0");

        SHA_Demo::StateVal concrete{
            0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au, 0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u,
        };

        const std::array<uint8_t, 64> block = {
            'a', 'b', 'c', 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0,   0,   0,   0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24,
        };

        auto scheduleValues = SHA_Demo::BuildScheduleReference(block, opt.steps);

        std::vector<BitFlow::Core::AST::Expr*> scheduleExprs;
        scheduleExprs.reserve(opt.steps > 16 ? opt.steps : 16);
        for (unsigned i = 0; i < 16; ++i) {
            scheduleExprs.push_back(b.Const(scheduleValues[i]));
        }

        if (opt.dumpFiles) {
            std::filesystem::create_directories(opt.outDir);
            SHA_Demo::WriteTextFile(opt.outDir / "run.txt", "ShaStepsDemo\n"
                                                            "factorize intentionally not included here due oscillation "
                                                            "in current BitFlow factorize/distribute rules.\n");
        }

        std::cout << "SHA step explorer\n";
        std::cout << "  steps       : " << opt.steps << "\n";
        std::cout << "  sha rules   : " << (opt.shaRules ? "on" : "off") << "\n";
        std::cout << "  schedule    : " << (opt.buildSchedule ? "full until step" : "W[0..15] only") << "\n";
        std::cout << "  dump files  : " << (opt.dumpFiles ? "on" : "off") << "\n";
        std::cout << "  out         : " << opt.outDir.string() << "\n\n";

        for (unsigned step = 0; step < opt.steps; ++step) {
            std::cout << "------------------------------------------------------------\n";
            std::cout << "step " << step << "  k=" << SHA_Demo::Hex32(SHA_Demo::kSha256[step]) << "\n";

            BitFlow::Core::AST::Expr* wExpr = nullptr;
            if (step < 16 || !opt.buildSchedule) {
                wExpr = scheduleExprs[step];
            } else {
                wExpr = SHA_Demo::BuildScheduleExprAt(b, scheduleExprs, step, opt.shaRules);
            }

            auto raw = SHA_Demo::BuildStep(b, symbolic, wExpr, SHA_Demo::kSha256[step]);
            auto t1rw = SHA_Demo::RewriteExpr(raw.t1, opt.shaRules);
            auto t2rw = SHA_Demo::RewriteExpr(raw.t2, opt.shaRules);
            auto rewritten = SHA_Demo::RewriteStepOutputs(raw, opt.shaRules);

            PrintExprLine("w" + std::to_string(step), wExpr, b.Names(), opt.consoleMax);
            PrintExprLine("t1", t1rw, b.Names(), opt.consoleMax);
            PrintExprLine("t2", t2rw, b.Names(), opt.consoleMax);
            PrintExprLine("a_next", rewritten.a, b.Names(), opt.consoleMax);
            PrintExprLine("e_next", rewritten.e, b.Names(), opt.consoleMax);

            const SHA_Demo::StateVal refNext =
                SHA_Demo::RefStep(concrete, SHA_Demo::kSha256[step], scheduleValues[step]);
            const SHA_Demo::StateVal bitflowNext =
                SHA_Demo::VerifyConcreteStep(concrete, SHA_Demo::kSha256[step], scheduleValues[step], opt.shaRules);

            std::cout << "    verify a: ref=" << SHA_Demo::Hex32(refNext.a)
                      << " bitflow=" << SHA_Demo::Hex32(bitflowNext.a)
                      << (refNext.a == bitflowNext.a ? "  OK" : "  FAIL") << "\n";
            std::cout << "    verify e: ref=" << SHA_Demo::Hex32(refNext.e)
                      << " bitflow=" << SHA_Demo::Hex32(bitflowNext.e)
                      << (refNext.e == bitflowNext.e ? "  OK" : "  FAIL") << "\n";

            if (opt.dumpFiles) {
                const std::string stepName = (step < 10 ? "step_0" : "step_") + std::to_string(step);
                const std::filesystem::path stepDir = opt.outDir / stepName;
                std::filesystem::create_directories(stepDir);

                SHA_Demo::DumpExprFile(stepDir, "w.txt", wExpr, b.Names());
                SHA_Demo::DumpExprFile(stepDir, "t1.txt", t1rw, b.Names());
                SHA_Demo::DumpExprFile(stepDir, "t2.txt", t2rw, b.Names());
                SHA_Demo::DumpExprFile(stepDir, "a_next.txt", rewritten.a, b.Names());
                SHA_Demo::DumpExprFile(stepDir, "e_next.txt", rewritten.e, b.Names());
                SHA_Demo::DumpStepSummaryFile(stepDir, step, SHA_Demo::kSha256[step], scheduleValues[step], refNext,
                                              bitflowNext);
            }

            if (refNext.a != bitflowNext.a || refNext.b != bitflowNext.b || refNext.c != bitflowNext.c ||
                refNext.d != bitflowNext.d || refNext.e != bitflowNext.e || refNext.f != bitflowNext.f ||
                refNext.g != bitflowNext.g || refNext.h != bitflowNext.h) {
                std::cerr << "Concrete verification mismatch at step " << step << "\n";
                return 2;
            }

            symbolic = rewritten;
            concrete = refNext;
        }

        std::cout << "\nfinal concrete state after " << opt.steps << " steps\n";
        std::cout << "  a=" << SHA_Demo::Hex32(concrete.a) << " b=" << SHA_Demo::Hex32(concrete.b)
                  << " c=" << SHA_Demo::Hex32(concrete.c) << " d=" << SHA_Demo::Hex32(concrete.d) << "\n";
        std::cout << "  e=" << SHA_Demo::Hex32(concrete.e) << " f=" << SHA_Demo::Hex32(concrete.f)
                  << " g=" << SHA_Demo::Hex32(concrete.g) << " h=" << SHA_Demo::Hex32(concrete.h) << "\n";

        if (opt.dumpFiles) {
            SHA_Demo::WriteTextFile(
                opt.outDir / "final_state.txt",
                "a=" + SHA_Demo::Hex32(concrete.a) + "\n" + "b=" + SHA_Demo::Hex32(concrete.b) + "\n" +
                    "c=" + SHA_Demo::Hex32(concrete.c) + "\n" + "d=" + SHA_Demo::Hex32(concrete.d) + "\n" +
                    "e=" + SHA_Demo::Hex32(concrete.e) + "\n" + "f=" + SHA_Demo::Hex32(concrete.f) + "\n" +
                    "g=" + SHA_Demo::Hex32(concrete.g) + "\n" + "h=" + SHA_Demo::Hex32(concrete.h) + "\n");
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 1;
    }
}
