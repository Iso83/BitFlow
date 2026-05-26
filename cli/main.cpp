#include <BitFlow/core/expression/ExprPrinter.h>
#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <BitFlow/io/ExprParser.h>
#include <iostream>
#include <string>

#ifndef BF_GIT_HASH
#define BF_GIT_HASH "unknown"
#endif

using namespace BitFlow::Core;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::IO;

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
                      << "git: " << BF_GIT_HASH << "\n"
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
                engine.SetDebugCallback([&](auto before, auto after, auto key) {
                    std::cout << "  " << key.value << "\n";

                    std::cout << "    " << ToString(&store, before, parsed.names, printOptions) << "\n";

                    std::cout << "    -> " << ToString(&store, after, parsed.names, printOptions) << "\n";
                });
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