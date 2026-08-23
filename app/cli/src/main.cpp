#include <BitFlow/engine/core/expression/ExprPrinter.h>
#include <BitFlow/engine/core/expression/ExprStore.h>
#include <BitFlow/engine/core/rules/RulePipeline.h>
#include <BitFlow/engine/core/rules/RuleTrace.h>
#include <BitFlow/engine/io/ExprParser.h>
#include <BitFlow/Version.h>
#include <iostream>
#include <string>

using namespace BitFlow::Engine::Core;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Rules;
using namespace BitFlow::Engine::IO;

static void PrintUsage() {
    std::cout <<
        R"(BitFlow CLI

Usage:
    bitflow "<expr>" [options]

Options:
    --rewrite      Apply rewrite pipeline
    --groups       Show explicit groups
    --ids          Show Expr IDs
    --ops          Show OpTypes
    --bitwidth     Show bit widths
    --trace        Show rewrite trace
)";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    std::string exprText;

    PrintOptions printOptions;

    bool doRewrite = false;
    bool trace = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--rewrite")
            doRewrite = true;
        else if (arg == "--groups")
            printOptions.explicitGroups = true;
        else if (arg == "--ids")
            printOptions.showExprIds = true;
        else if (arg == "--ops")
            printOptions.showOpTypes = true;
        else if (arg == "--bitwidth")
            printOptions.showBitWidth = true;
        else if (arg == "--trace")
            trace = true;
        else if (arg == "--version") {
            std::cout << "BitFlow CLI\n"
                      << "git: " << BitFlow::GitHash << "\n"
                      << "build: " << __DATE__ << " " << __TIME__ << "\n";

            return 0;
        } else
            exprText = arg;
    }

    try {
        ExprStore store;

        auto parsed = Parse(&store, exprText);

        std::cout << "Input:\n";
        std::cout << "    " << ToString(parsed.root, parsed.names, printOptions) << "\n";

        if (doRewrite) {

            RuleEngine engine;

            engine.Merge(BuildNormalize());
            engine.Merge(BuildSimplifyArithmetic());
            engine.Merge(BuildSimplifyBitwise());
            engine.Merge(BuildFactorizeArithmetic());
            engine.Merge(BuildFactorizeBitwise());

            if (trace) {
                std::cout << "\nTrace:\n";
                AttachConsoleTrace(engine, parsed.names, printOptions);
            }

            ExprRef result = ExprRef(&store, engine.ApplyRecursive(&store, parsed.root.id));

            std::cout << "\nRewrite:\n";
            std::cout << "    " << ToString(result, parsed.names, printOptions) << "\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "\nError:\n";
        std::cerr << "    " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
