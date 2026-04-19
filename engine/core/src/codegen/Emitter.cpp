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

    return ApplyMask(expr, bitWidth);
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
        if (!declared.insert(st.name).second) {
            ss << "    " << st.name << " = " << ApplyMask(st.expr, bitWidth) << ";\n";
            continue;
        }
        ss << "    " << ctype << " " << st.name << " = " << ApplyMask(st.expr, bitWidth) << ";\n";
    }

    ss << "    return " << ApplyMask(prog.result.empty() ? std::string("0") : prog.result, bitWidth) << ";\n";
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

    for (const auto& st : prog.statements)
        ss << "    " << ctype << " " << st.name << " = " << ApplyMask(st.expr, bitWidth) << ";\n";

    ss << "    EvalResult r{};\n";
    for (size_t i = 0; i < prog.results.size(); ++i)
        ss << "    r.out" << (i + 1) << " = " << ApplyMask(prog.results[i], bitWidth) << ";\n";
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
