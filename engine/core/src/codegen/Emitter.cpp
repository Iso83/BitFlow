#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/codegen/TypeMap.h>
#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace BitFlow::Core::Codegen {

using namespace AST;

namespace {

static std::string MakeMask(uint32_t bw) {
    if (bw == 32U)
        return "0xffffffffu";
    if (bw == 64U)
        return "~0ull";
    return "((1ull << " + std::to_string(bw) + ") - 1)";
}

static std::string ApplyMask(const std::string& expr, uint32_t bitWidth) {
    if (bitWidth > 64U)
        return expr;
    return "((" + expr + ") & " + MakeMask(bitWidth) + ")";
}

static std::string ZeroValueExpr(uint32_t bitWidth) {
    if (bitWidth > 64U)
        return "bf_uint(0ull, " + std::to_string(bitWidth) + ")";
    return "0";
}

static bool IsWordChar(char c) {
    const bool isLower = (c >= 'a' && c <= 'z');
    const bool isUpper = (c >= 'A' && c <= 'Z');
    const bool isDigit = (c >= '0' && c <= '9');
    return isLower || isUpper || isDigit || c == '_';
}

static void ReplaceIdentifierToken(std::string& text, const std::string& from, const std::string& to) {
    size_t pos = 0;
    while ((pos = text.find(from, pos)) != std::string::npos) {
        const bool leftBoundary = (pos == 0) || !IsWordChar(text[pos - 1]);
        const size_t rightPos = pos + from.size();
        const bool rightBoundary = (rightPos >= text.size()) || !IsWordChar(text[rightPos]);
        if (leftBoundary && rightBoundary) {
            text.replace(pos, from.size(), to);
            pos += to.size();
        } else {
            pos += from.size();
        }
    }
}

static std::string InlineSsaResult(const SsaProgram& prog, uint32_t bitWidth) {
    std::unordered_map<std::string, std::string> env;

    for (const auto& st : prog.statements) {
        std::string rhs = st.expr;
        for (const auto& [name, value] : env)
            ReplaceIdentifierToken(rhs, name, value);

        env[st.name] = "(" + ApplyMask(rhs, bitWidth) + ")";
    }

    std::string expr = prog.result.empty() ? "0" : prog.result;
    for (const auto& [name, value] : env)
        ReplaceIdentifierToken(expr, name, value);

    expr = ApplyMask(expr, bitWidth);
    ReplaceIdentifierToken(expr, "rotl", "bf_rotl");
    ReplaceIdentifierToken(expr, "rotr", "bf_rotr");
    return expr;
}

static void CollectVars(const Expr* e, std::set<uint32_t>& out) {
    if (!e)
        return;

    if (e->op == OpType::Var)
        out.insert(e->id.value());

    for (const Expr* in : e->inputs)
        CollectVars(in, out);
}

} // namespace

std::string EmitCRuntimeSupport(uint32_t bitWidth) {
    std::ostringstream ss;
    ss << "#include <cstdint>\n";

    if (bitWidth > 64U) {
        ss << "#include <BitFlow/core/bitvector/BitVector.h>\n";
        ss << "using bf_uint = BitFlow::Core::BitVector::bf_uint;\n";
    }

    ss << "\n";
    ss << "// BitFlow generated rotate contract:\n";
    ss << "// - bitWidth <= 32: bf_rotl/bf_rotr operate on uint32_t\n";
    ss << "// - bitWidth <= 64: bf_rotl/bf_rotr operate on uint64_t\n";
    ss << "// - bitWidth > 64: bf_rotl/bf_rotr operate on bf_uint (C++ runtime type)\n";

    if (bitWidth <= 32U) {
        ss << "[[maybe_unused]] static inline uint32_t bf_rotl(uint32_t value, uint32_t shift) {\n";
        ss << "    shift &= 31u;\n";
        ss << "    if (shift == 0u)\n";
        ss << "        return value;\n";
        ss << "    return static_cast<uint32_t>((value << shift) | (value >> ((32u - shift) & 31u)));\n";
        ss << "}\n\n";

        ss << "[[maybe_unused]] static inline uint32_t bf_rotr(uint32_t value, uint32_t shift) {\n";
        ss << "    shift &= 31u;\n";
        ss << "    if (shift == 0u)\n";
        ss << "        return value;\n";
        ss << "    return static_cast<uint32_t>((value >> shift) | (value << ((32u - shift) & 31u)));\n";
        ss << "}\n\n";
    } else if (bitWidth <= 64U) {
        ss << "[[maybe_unused]] static inline uint64_t bf_rotl(uint64_t value, uint32_t shift) {\n";
        ss << "    shift &= 63u;\n";
        ss << "    if (shift == 0u)\n";
        ss << "        return value;\n";
        ss << "    return (value << shift) | (value >> ((64u - shift) & 63u));\n";
        ss << "}\n\n";

        ss << "[[maybe_unused]] static inline uint64_t bf_rotr(uint64_t value, uint32_t shift) {\n";
        ss << "    shift &= 63u;\n";
        ss << "    if (shift == 0u)\n";
        ss << "        return value;\n";
        ss << "    return (value >> shift) | (value << ((64u - shift) & 63u));\n";
        ss << "}\n\n";
    } else {
        ss << "[[maybe_unused]] static inline bf_uint bf_rotl(const bf_uint& value, uint32_t shift) {\n";
        ss << "    return value.RotL(shift);\n";
        ss << "}\n\n";
        ss << "[[maybe_unused]] static inline bf_uint bf_rotl(const bf_uint& value, const bf_uint& shift) {\n";
        ss << "    return value.RotL(shift.ToUint32());\n";
        ss << "}\n\n";

        ss << "[[maybe_unused]] static inline bf_uint bf_rotr(const bf_uint& value, uint32_t shift) {\n";
        ss << "    return value.RotR(shift);\n";
        ss << "}\n\n";
        ss << "[[maybe_unused]] static inline bf_uint bf_rotr(const bf_uint& value, const bf_uint& shift) {\n";
        ss << "    return value.RotR(shift.ToUint32());\n";
        ss << "}\n\n";
    }

    return ss.str();
}

std::string EmitCExpr(const Expr* root, uint32_t bitWidth) {
    if (!root || bitWidth == 0U)
        return "0";

    // Hard flow: AST -> SSA(+PerfPass in BuildSSA) -> C emission.
    const SsaProgram prog = BuildSSA(root, bitWidth);
    return InlineSsaResult(prog, bitWidth);
}

std::string EmitCFunction(const Expr* root, uint32_t bitWidth) {
    if (!root || bitWidth == 0U)
        return GetCType(bitWidth) + " eval() { return 0; }";

    // Hard flow: AST -> SSA(+PerfPass in BuildSSA) -> C emission.
    const SsaProgram prog = BuildSSA(root, bitWidth);

    std::set<uint32_t> vars;
    CollectVars(root, vars);

    std::ostringstream ss;
    const std::string ctype = GetCType(bitWidth);

    ss << ctype << " eval(";
    bool first = true;
    for (uint32_t id : vars) {
        if (!first)
            ss << ", ";
        ss << ctype << " v" << id;
        first = false;
    }
    ss << ") {\n";

    std::unordered_set<std::string> declared;
    for (const auto& st : prog.statements) {
        std::string stmtExpr = ApplyMask(st.expr, bitWidth);
        ReplaceIdentifierToken(stmtExpr, "rotl", "bf_rotl");
        ReplaceIdentifierToken(stmtExpr, "rotr", "bf_rotr");

        if (!declared.insert(st.name).second) {
            ss << "    " << st.name << " = " << stmtExpr << ";\n";
            continue;
        }
        ss << "    " << ctype << " " << st.name << " = " << stmtExpr << ";\n";
    }

    std::string resultExpr = ApplyMask(prog.result.empty() ? std::string("0") : prog.result, bitWidth);
    ReplaceIdentifierToken(resultExpr, "rotl", "bf_rotl");
    ReplaceIdentifierToken(resultExpr, "rotr", "bf_rotr");
    ss << "    return " << resultExpr << ";\n";
    ss << "}\n\n";

    ss << ctype << " f(";
    first = true;
    for (uint32_t id : vars) {
        if (!first)
            ss << ", ";
        ss << ctype << " v" << id;
        first = false;
    }
    ss << ") {\n";
    ss << "    return eval(";
    first = true;
    for (uint32_t id : vars) {
        if (!first)
            ss << ", ";
        ss << "v" << id;
        first = false;
    }
    ss << ");\n";
    ss << "}";

    return ss.str();
}

std::string EmitCFunctionMulti(const std::vector<const Expr*>& roots, uint32_t bitWidth) {
    const std::string ctype = GetCType(bitWidth);
    if (roots.empty() || bitWidth == 0U)
        return "struct EvalResult {\n};\n\nEvalResult eval() {\n    return EvalResult{};\n}\n\nEvalResult f() {\n    "
               "return eval();\n}";

    const SsaProgram prog = BuildSSA(roots, bitWidth);

    std::set<uint32_t> vars;
    for (const Expr* root : roots)
        CollectVars(root, vars);

    std::ostringstream ss;
    ss << "struct EvalResult {\n";
    for (size_t i = 0; i < roots.size(); ++i)
        ss << "    " << ctype << " out" << (i + 1) << ";\n";
    ss << "};\n\n";

    ss << "EvalResult eval(";
    bool first = true;
    for (uint32_t id : vars) {
        if (!first)
            ss << ", ";
        ss << ctype << " v" << id;
        first = false;
    }
    ss << ") {\n";

    for (const auto& st : prog.statements) {
        std::string stmtExpr = ApplyMask(st.expr, bitWidth);
        ReplaceIdentifierToken(stmtExpr, "rotl", "bf_rotl");
        ReplaceIdentifierToken(stmtExpr, "rotr", "bf_rotr");
        ss << "    " << ctype << " " << st.name << " = " << stmtExpr << ";\n";
    }

    ss << "    EvalResult r{";
    for (size_t i = 0; i < prog.results.size(); ++i) {
        if (i != 0)
            ss << ", ";
        ss << ZeroValueExpr(bitWidth);
    }
    ss << "};\n";
    for (size_t i = 0; i < prog.results.size(); ++i) {
        std::string resultExpr = ApplyMask(prog.results[i], bitWidth);
        ReplaceIdentifierToken(resultExpr, "rotl", "bf_rotl");
        ReplaceIdentifierToken(resultExpr, "rotr", "bf_rotr");
        ss << "    r.out" << (i + 1) << " = " << resultExpr << ";\n";
    }
    ss << "    return r;\n";
    ss << "}\n\n";

    ss << "EvalResult f(";
    first = true;
    for (uint32_t id : vars) {
        if (!first)
            ss << ", ";
        ss << ctype << " v" << id;
        first = false;
    }
    ss << ") {\n";
    ss << "    return eval(";
    first = true;
    for (uint32_t id : vars) {
        if (!first)
            ss << ", ";
        ss << "v" << id;
        first = false;
    }
    ss << ");\n";
    ss << "}";

    return ss.str();
}

} // namespace BitFlow::Core::Codegen
