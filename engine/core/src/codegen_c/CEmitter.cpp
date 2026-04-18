#include <BitFlow/core/codegen_c/CEmitter.h>
#include <BitFlow/core/codegen_ssa/SsaBuilder.h>

#include <BitFlow/core/ast/Expression.h>

#include <set>
#include <sstream>

namespace BitFlow::Core::Codegen {

using namespace AST;

static std::string MakeMask(uint32_t bw) {
    if (bw == 64)
        return "~0ull";

    return "((1ull << " + std::to_string(bw) + ") - 1ull)";
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
    // SSA build
    SsaProgram prog = BuildSSA(root, bitWidth);

    // vars verzamelen
    std::set<uint32_t> vars;
    CollectVars(root, vars);

    std::ostringstream ss;

    // signature
    ss << "uint64_t f(";

    bool first = true;
    for (auto id : vars) {
        if (!first)
            ss << ", ";
        ss << "uint64_t v" << id;
        first = false;
    }

    ss << ") {\n";

    // locals
    for (const auto& st : prog.statements)
        ss << "    uint64_t " << st.name << " = " << st.expr << ";\n";

    // return
    std::string mask = MakeMask(bitWidth);
    const std::string result = prog.result.empty() ? "0ull" : prog.result;

    ss << "    return (" << result << ") & " << mask << ";\n";
    ss << "}";

    return ss.str();
}

} // namespace BitFlow::Core::Codegen
