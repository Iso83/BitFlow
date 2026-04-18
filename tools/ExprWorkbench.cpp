#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <BitFlow/core/eval/ConstantDetect.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <BitFlow/io/ExprParser.h>
#include <BitFlow/io/ExprPrinter.h>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

using namespace BitFlow;
using Expr = Core::AST::Expr;

struct CliOptions {
    std::string expr;
    uint32_t bitWidth = 32;

    bool normalize = false;
    bool simplify = false;
    bool factorize = false;

    bool ssa = false;
    bool emitC = false;
    bool emitFunc = false;
    bool eval = false;
    bool trace = false;
};

const char* EvalStatusToString(Core::Eval::EvalStatus status) {
    using Core::Eval::EvalStatus;

    switch (status) {
    case EvalStatus::Success:
        return "Success";
    case EvalStatus::NotConstant:
        return "NotConstant";
    case EvalStatus::DivisionByZero:
        return "DivisionByZero";
    case EvalStatus::ModuloByZero:
        return "ModuloByZero";
    case EvalStatus::InvalidBitWidth:
        return "InvalidBitWidth";
    case EvalStatus::UnsupportedOp:
    default:
        return "UnsupportedOp";
    }
}

void PrintUsage() {
    std::cerr << "Usage:\n";
    std::cerr << "  bitflow_expr --expr \"...\" --bitwidth 32 [flags]\n\n";
    std::cerr << "Flags:\n";
    std::cerr << "  --normalize\n";
    std::cerr << "  --simplify\n";
    std::cerr << "  --factorize\n";
    std::cerr << "  --ssa\n";
    std::cerr << "  --emit-c\n";
    std::cerr << "  --emit-func\n";
    std::cerr << "  --eval\n";
    std::cerr << "  --trace\n";
}

bool ParseUInt32(const std::string& text, uint32_t& out) {
    try {
        const unsigned long value = std::stoul(text);
        if (value > 0xfffffffful)
            return false;
        out = static_cast<uint32_t>(value);
        return true;
    } catch (...) {
        return false;
    }
}

CliOptions ParseArgs(int argc, char** argv) {
    CliOptions opt;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--expr") {
            if (i + 1 >= argc)
                throw std::runtime_error("Missing value for --expr");
            opt.expr = argv[++i];
            continue;
        }

        if (arg == "--bitwidth") {
            if (i + 1 >= argc)
                throw std::runtime_error("Missing value for --bitwidth");

            uint32_t bw = 0;
            if (!ParseUInt32(argv[++i], bw))
                throw std::runtime_error("Invalid --bitwidth value");

            opt.bitWidth = bw;
            continue;
        }

        if (arg == "--normalize") {
            opt.normalize = true;
            continue;
        }
        if (arg == "--simplify") {
            opt.simplify = true;
            continue;
        }
        if (arg == "--factorize") {
            opt.factorize = true;
            continue;
        }
        if (arg == "--ssa") {
            opt.ssa = true;
            continue;
        }
        if (arg == "--emit-c") {
            opt.emitC = true;
            continue;
        }
        if (arg == "--emit-func") {
            opt.emitFunc = true;
            continue;
        }
        if (arg == "--eval") {
            opt.eval = true;
            continue;
        }
        if (arg == "--trace") {
            opt.trace = true;
            continue;
        }

        throw std::runtime_error("Unknown argument: " + arg);
    }

    if (opt.expr.empty())
        throw std::runtime_error("Missing --expr");

    return opt;
}

void PrintSectionHeader(const char* title) {
    std::cout << "[" << title << "]\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
        const CliOptions opt = ParseArgs(argc, argv);

        auto parsed = IO::Parse(opt.expr);

        PrintSectionHeader("parsed");
        std::cout << IO::ToString(parsed.root, parsed.idToName) << "\n\n";

        const bool hasStageSelection = opt.normalize || opt.simplify || opt.factorize;
        const bool runNormalize = hasStageSelection ? opt.normalize : true;
        const bool runSimplify = hasStageSelection ? opt.simplify : true;
        const bool runFactorize = hasStageSelection ? opt.factorize : true;

        Core::Rules::RuleEngine engine;

        if (runNormalize)
            Core::Rules::Add_Normalize_Rules(engine);

        if (runSimplify) {
            Core::Rules::Add_Simplify_Bitwise_Rules(engine);
            Core::Rules::Add_Simplify_Arithmetic_Rules(engine);
        }

        if (runFactorize) {
            Core::Rules::Add_Factorize_Bitwise_Rules(engine);
            Core::Rules::Add_Factorize_Arithmetic_Rules(engine);
        }

        if (opt.trace) {
            engine.SetDebugCallback([&](const Expr* before, const Expr* after, Core::Rules::RuleId id) {
                std::cout << "[trace] [" << static_cast<int>(id) << "] " << IO::ToString(before, parsed.idToName)
                          << " -> " << IO::ToString(after, parsed.idToName) << "\n";
            });
        }

        Expr* rewritten = engine.ApplyUntilStable(parsed.root);

        PrintSectionHeader("rewritten");
        std::cout << IO::ToString(rewritten, parsed.idToName) << "\n\n";

        if (opt.ssa) {
            Core::Codegen::SsaProgram ssa = Core::Codegen::BuildSSA(rewritten, opt.bitWidth);
            PrintSectionHeader("ssa");
            for (const auto& st : ssa.statements)
                std::cout << st.name << " = " << st.expr << "\n";
            std::cout << "result = " << (ssa.result.empty() ? "0" : ssa.result) << "\n\n";
        }

        if (opt.emitC) {
            PrintSectionHeader("c-expr");
            std::cout << Core::Codegen::EmitCExpr(rewritten, opt.bitWidth) << "\n\n";
        }

        if (opt.emitFunc) {
            PrintSectionHeader("c-func");
            std::cout << Core::Codegen::EmitCFunction(rewritten, opt.bitWidth) << "\n\n";
        }

        if (opt.eval) {
            PrintSectionHeader("eval");
            if (!Core::Eval::IsFullyConstant(rewritten)) {
                std::cout << "NotConstant\n\n";
            } else {
                const auto result = Core::Eval::EvaluateConstant(rewritten, opt.bitWidth);
                if (result.status == Core::Eval::EvalStatus::Success)
                    std::cout << result.value << "\n\n";
                else
                    std::cout << EvalStatusToString(result.status) << "\n\n";
            }
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        PrintUsage();
        return 1;
    }
}
