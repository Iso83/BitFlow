#include <BitFlow/io/ExprEvaluator.h>

#include <BitFlow/io/ExprParser.h>
#include <exception>

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
    EvaluatePrintResult out{};
    try {
        const ParseResult parsed = Parse(input);
        out.eval = Core::Eval::EvaluateConstant(parsed.root, bitWidth);
    } catch (const std::exception& ex) {
        out.parseOk = false;
        out.parseError = ex.what();
        out.text = "result: error: parse failed: " + out.parseError;
        return out;
    }

    if (out.eval.status == Core::Eval::EvalStatus::Success) {
        out.text = "result: success, value=" + std::to_string(out.eval.value) +
                   ", bitwidth=" + std::to_string(bitWidth);
    }
    else
        out.text = std::string{"result: "} + StatusMessage(out.eval.status);

    return out;
}

} // namespace BitFlow::IO
