#include <BitFlow/io/ExprEvaluator.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

bool TryParseBitWidth(const std::string& token, uint32_t& outBitWidth) {
    try {
        size_t consumed = 0;
        const unsigned long parsed = std::stoul(token, &consumed, 10);
        if (consumed != token.size())
            return false;
        if (parsed == 0 || parsed > 64)
            return false;
        outBitWidth = static_cast<uint32_t>(parsed);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace

int main() {
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line))
            break;

        if (line.empty())
            continue;

        std::istringstream in(line);
        std::string bitWidthToken;
        if (!(in >> bitWidthToken)) {
            std::cout << "error: expected '<bitwidth> <expression>'" << "\n";
            continue;
        }
        uint32_t bitWidth = 0;
        if (!TryParseBitWidth(bitWidthToken, bitWidth)) {
            std::cout << "result: error: invalid bitwidth (must be in range 1..64)" << "\n";
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
