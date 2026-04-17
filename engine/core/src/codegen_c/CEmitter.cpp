#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/codegen_c/CEmitter.h>
#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace BitFlow::Core::CodegenC {

namespace {

static constexpr const char* kDefaultType = "uint64_t";

static bool IsIdentifierStart(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool IsIdentifierChar(char c) {
    return IsIdentifierStart(c) || (c >= '0' && c <= '9');
}

static bool IsValidIdentifier(const std::string& name) {
    if (name.empty())
        return false;

    if (!IsIdentifierStart(name.front()))
        return false;

    return std::all_of(name.begin() + 1, name.end(), IsIdentifierChar);
}

static std::string MakeVarName(uint32_t id) {
    return "v" + std::to_string(id);
}

static void CollectVars(const AST::Expr* e, std::set<uint32_t>& out) {
    if (!e)
        return;

    if (e->op == AST::OpType::Var)
        out.insert(e->id.value());

    for (const AST::Expr* input : e->inputs)
        CollectVars(input, out);
}

} // namespace

std::vector<CParameter> BuildCParameters(const AST::Expr* root, const std::map<uint32_t, std::string>& names) {
    std::set<uint32_t> ids;
    CollectVars(root, ids);

    std::vector<CParameter> params;
    params.reserve(ids.size());
    for (const uint32_t id : ids) {
        const auto it = names.find(id);
        const std::string resolvedName = (it != names.end() && IsValidIdentifier(it->second)) ? it->second
                                                                                                 : MakeVarName(id);
        params.push_back({id, kDefaultType, resolvedName});
    }

    return params;
}

std::vector<CLocal> BuildCLocals(const AST::Expr* root, uint32_t bitWidth) {
    std::vector<CLocal> locals;
    const Codegen::SsaProgram ssa = Codegen::BuildSSA(root, bitWidth);
    locals.reserve(ssa.statements.size());

    for (const auto& statement : ssa.statements)
        locals.push_back({kDefaultType, statement.name, statement.expr});

    return locals;
}

CFunction BuildCFunction(const AST::Expr* root, uint32_t bitWidth, const std::string& functionName,
                         const std::map<uint32_t, std::string>& names) {
    CFunction function{};
    function.returnType = kDefaultType;
    function.functionName = functionName.empty() ? "f" : functionName;
    function.parameters = BuildCParameters(root, names);
    function.locals = BuildCLocals(root, bitWidth);

    const Codegen::SsaProgram ssa = Codegen::BuildSSA(root, bitWidth);
    if (!ssa.result.empty())
        function.returnExpr = ssa.result;
    else
        function.returnExpr = Codegen::EmitCExpr(root, bitWidth);

    return function;
}

std::string EmitCFunction(const CFunction& function) {
    std::string out;
    out += function.returnType + " " + function.functionName + "(";
    for (size_t i = 0; i < function.parameters.size(); ++i) {
        if (i > 0)
            out += ", ";
        out += function.parameters[i].type + " " + function.parameters[i].name;
    }
    out += ") {\n";

    for (const auto& local : function.locals)
        out += "    " + local.type + " " + local.name + " = " + local.expr + ";\n";

    out += "    return " + function.returnExpr + ";\n";
    out += "}";
    return out;
}

} // namespace BitFlow::Core::CodegenC
