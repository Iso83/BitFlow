#include <BitFlow/io/ExprEvaluator.h>

#include <BitFlow/io/ExprParser.h>

namespace BitFlow::IO {
namespace {

const char* StatusName(Core::Eval::EvalStatus status) {
    using Status = Core::Eval::EvalStatus;

    switch (status) {
    case Status::Success:
        return "success";
    case Status::NotConstant:
        return "not-constant";
    case Status::InvalidBitWidth:
        return "invalid-bitwidth";
    case Status::DivisionByZero:
        return "division-by-zero";
    case Status::ModuloByZero:
        return "modulo-by-zero";
    case Status::UnsupportedOp:
        return "unsupported-op";
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
        out.text = std::to_string(eval.value);
    else
        out.text = StatusName(eval.status);

    return out;
}

} // namespace BitFlow::IO
