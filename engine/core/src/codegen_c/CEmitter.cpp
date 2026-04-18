#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/codegen/TypeMap.h>
#include <BitFlow/core/codegen_c/CEmitter.h>
#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <set>
#include <sstream>

namespace BitFlow::Core::Codegen {

using namespace AST;

static std::string MakeMask(uint32_t bw) {
    if (bw == 64)
        return "~0ull";

    return "((1ull << " + std::to_string(bw) + ") - 1)";
}

// verzamel variabelen (vX)
static void CollectVars(const Expr* e, std::set<uint32_t>& out) {
    if (!e)
        return;

    if (e->op == OpType::Var) {
        out.insert(e->id.value());
        return;
    }

    for (auto* in : e->inputs)
        CollectVars(in, out);
}

std::string EmitCFunction(const Expr* root, uint32_t bitWidth) {
    SsaProgram prog = BuildSSA(root, bitWidth);

    std::set<uint32_t> vars;
    CollectVars(root, vars);

    std::ostringstream ss;
    const std::string mask = MakeMask(bitWidth);
    const std::string cType = GetCType(bitWidth);

    ss << cType << " eval(";

    bool first = true;
    for (auto id : vars) {
        if (!first)
            ss << ", ";
        ss << cType << " v" << id;
        first = false;
    }

    ss << ") {\n";

    for (const auto& st : prog.statements)
        ss << "    " << cType << " " << st.name << " = ((" << st.expr << ")) & " << mask << ";\n";

    const std::string result = prog.result.empty() ? "0" : prog.result;
    ss << "    return (" << result << ") & " << mask << ";\n";
    ss << "}\n\n";

    ss << cType << " f(";
    first = true;
    for (auto id : vars) {
        if (!first)
            ss << ", ";
        ss << cType << " v" << id;
        first = false;
    }
    ss << ") {\n";
    ss << "    return eval(";
    first = true;
    for (auto id : vars) {
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
