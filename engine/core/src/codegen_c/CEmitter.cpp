#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/codegen_c/CEmitter.h>
#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <sstream>

namespace BitFlow::Core::Codegen {

namespace {

static std::string MakeMask(uint32_t bitWidth) {
    if (bitWidth == 64)
        return "0xffffffffffffffffull";

    return "((1ull << " + std::to_string(bitWidth) + ") - 1ull)";
}

static std::string ApplyMask(const std::string& expr, uint32_t bitWidth) {
    return "((" + expr + ") & " + MakeMask(bitWidth) + ")";
}

} // namespace

std::string EmitCFunction(const AST::Expr* root, uint32_t bitWidth) {
    std::ostringstream out;
    out << "uint64_t f(";
    out << EmitCParamList(root, bitWidth);
    out << ") {\n";

    const SsaProgram ssa = BuildSSA(root, bitWidth);
    for (const auto& stmt : ssa.statements)
        out << "    uint64_t " << stmt.name << " = " << stmt.expr << ";\n";

    std::string result;
    if (!ssa.results.empty())
        result = ssa.results.back();
    else if (!ssa.result.empty())
        result = ssa.result;
    else
        result = EmitCExpr(root, bitWidth);

    out << "    return " << ApplyMask(result, bitWidth) << ";\n";
    out << "}";
    return out.str();
}

} // namespace BitFlow::Core::Codegen
