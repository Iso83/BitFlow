#pragma once

#include <BitFlow/core/eval/ConstEvaluator.h>
#include <cstdint>
#include <string>

namespace BitFlow::IO {

struct EvaluatePrintResult {
    Core::Eval::EvalResult eval;
    std::string text;
    bool parseOk = true;
    std::string parseError;
};

// Convenience entrypoint for parse -> evaluate -> print result.
EvaluatePrintResult ParseEvaluatePrint(const std::string& input, uint32_t bitWidth);

} // namespace BitFlow::IO
