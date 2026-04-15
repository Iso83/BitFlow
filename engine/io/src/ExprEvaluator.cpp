#include <BitFlow/io/ExprEvaluator.h>

#include <BitFlow/io/ExprParser.h>

namespace BitFlow::IO {
namespace {

const char* StatusMessage(Core::Eval::EvalStatus status) {
    using Status = Core::Eval::EvalStatus;

    switch (status) {
    case Status::Success:
        return "success";
    case Status::NotConstant:
        return "error: expression is not fully constant";
    case Status::InvalidBitWidth:
        return "error: invalid bitwidth (must be in range 1..64)";
    case Status::DivisionByZero:
        return "error: division by zero";
    case Status::ModuloByZero:
        return "error: modulo by zero";
    case Status::UnsupportedOp:
        return "error: unsupported operation for constant evaluation";
    }

    return "unknown";
}

} // namespace

EvaluatePrintResult ParseEvaluatePrint(const std::string& input, uint32_t bitWidth) {
    const ParseResult parsed = Parse(input);
    const Core::Eval::EvalResult eval = Core::Eval::EvaluateConstant(parsed.root, bitWidth);

    EvaluatePrintResult out{};
    out.eval = eval;

    if (eval.status == Core::Eval::EvalStatus::Success)
        out.text = "result: success, value=" + std::to_string(eval.value);
    else
        out.text = std::string{"result: "} + StatusMessage(eval.status);

    return out;
}

} // namespace BitFlow::IO
