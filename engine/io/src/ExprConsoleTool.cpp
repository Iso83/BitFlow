#include <BitFlow/io/ExprEvaluator.h>
#include <iostream>
#include <string>

int main() {
    const uint32_t bitWidth = 32;
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line))
            break;

        if (line.empty())
            continue;

        const auto evaluated = BitFlow::IO::ParseEvaluatePrint(line, bitWidth);
        std::cout << evaluated.text << "\n";
    }

    return 0;
}
