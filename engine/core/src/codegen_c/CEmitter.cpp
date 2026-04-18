#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/bitvector/BitVector.h>
#include <BitFlow/core/codegen/TypeMap.h>
#include <BitFlow/core/codegen_c/CEmitter.h>
#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <set>
#include <sstream>

namespace BitFlow::Core::Codegen {

using namespace AST;

static std::string MakeMask(uint32_t bw) {
    if (bw == 32)
        return "0xffffffffu";

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
    auto ctype = GetCType(bitWidth);
    const bool useBfUint = (bitWidth > 64U);
    const std::string mask = MakeMask(bitWidth);

    ss << ctype << " eval(";

    bool first = true;
    for (auto id : vars) {
        if (!first)
            ss << ", ";
        ss << ctype << " v" << id;
        first = false;
    }

    ss << ") {\n";

    for (const auto& st : prog.statements) {
        if (useBfUint)
            ss << "    " << ctype << " " << st.name << " = " << st.expr << ";\n";
        else
            ss << "    " << ctype << " " << st.name << " = (" << ctype << ")((" << st.expr << ") & (" << ctype
               << ")(" << mask << "));\n";
    }

    const std::string result = prog.result.empty()
                                   ? (useBfUint ? "bf_uint(0ull, " + std::to_string(bitWidth) + ")" : "0")
                                   : prog.result;

    if (useBfUint)
        ss << "    return " << result << ";\n";
    else
        ss << "    return (" << ctype << ")((" << result << ") & (" << ctype << ")(" << mask << "));\n";
    ss << "}\n\n";

    ss << ctype << " f(";
    first = true;
    for (auto id : vars) {
        if (!first)
            ss << ", ";
        ss << ctype << " v" << id;
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
