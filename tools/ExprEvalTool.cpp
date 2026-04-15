#include <BitFlow/core/eval/ConstantEval.h>
#include <BitFlow/core/eval/ConstantDetect.h>
#include <BitFlow/io/ExprParser.h>

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>

namespace {

const char* ToStatusString(BitFlow::Core::Eval::EvalStatus status) {
    using BitFlow::Core::Eval::EvalStatus;

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
        return "UnsupportedOp";
    default:
        return "UnsupportedOp";
    }
}

bool ReadInput(std::string line, uint32_t& bitWidth, std::string& expression) {
    std::istringstream iss(line);

    uint64_t bitWidthRaw = 0;
    if (!(iss >> bitWidthRaw))
        return false;

    std::string rest;
    std::getline(iss, rest);
    if (!rest.empty() && rest[0] == ' ')
        rest.erase(0, 1);

    if (rest.empty())
        return false;

    bitWidth = static_cast<uint32_t>(bitWidthRaw);
    expression = std::move(rest);
    return true;
}

} // namespace

int main() {
    std::string line;

    while (std::getline(std::cin, line)) {
        uint32_t bitWidth = 0;
        std::string expression;

        if (!ReadInput(line, bitWidth, expression)) {
            std::cout << "error: InvalidBitWidth\n";
            continue;
        }

        auto parsed = BitFlow::IO::Parse(expression);
        if (!BitFlow::Core::Eval::IsFullyConstant(parsed.root)) {
            std::cout << "error: NotConstant\n";
            continue;
        }

        BitFlow::Core::Eval::EvalResult result = BitFlow::Core::Eval::EvaluateConstant(parsed.root, bitWidth);

        if (result.status == BitFlow::Core::Eval::EvalStatus::Success) {
            std::cout << "result:\n";
            std::cout << result.value << "\n";
            continue;
        }

        std::cout << "error: " << ToStatusString(result.status) << "\n";
    }

    return 0;
}
