#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <BitFlow/io/ExprParser.h>
#include <BitFlow/io/ExprPrinter.h>
#include <iostream>
#include <string>

using namespace BitFlow::Core::Rules;
using namespace BitFlow::Core::AST;

int main() {
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line))
            break;

        if (line.empty())
            continue;

        auto parsed = BitFlow::IO::Parse(line);

        RuleEngine engine;
        engine.SetDebugCallback([&](const Expr* before, const Expr* after, RuleId id) {
            std::cout << "[" << (int)id << "] " << BitFlow::IO::ToString(before, parsed.idToName) << " -> "
                      << BitFlow::IO::ToString(after, parsed.idToName) << "\n";
        });

        Add_Bitwise_Simplify_Pipeline(engine);

        Expr* result = engine.ApplyUntilStable(parsed.root);
        std::cout << BitFlow::IO::ToString(result, parsed.idToName) << "\n";
    }

    return 0;
}
