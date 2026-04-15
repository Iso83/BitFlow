#include <BitFlow/io/ExprEvaluator.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>

int main() {
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line))
            break;

        if (line.empty())
            continue;

        std::istringstream in(line);
        uint32_t bitWidth = 0;
        if (!(in >> bitWidth)) {
            std::cout << "error: expected '<bitwidth> <expression>'" << "\n";
            continue;
        }

        std::string expr;
        std::getline(in >> std::ws, expr);
        if (expr.empty()) {
            std::cout << "error: missing expression after bitwidth" << "\n";
            continue;
        }

        const auto evaluated = BitFlow::IO::ParseEvaluatePrint(expr, bitWidth);
        std::cout << evaluated.text << "\n";
    }

    return 0;
}
