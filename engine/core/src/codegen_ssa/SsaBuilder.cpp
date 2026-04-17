#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <unordered_map>

namespace BitFlow::Core::Codegen::SSA {
namespace {

SsaValue MakeInvalid() {
    return SsaValue{SsaValueKind::Invalid, 0, 0};
}

SsaValue MakeConst(uint64_t value) {
    return SsaValue{SsaValueKind::Constant, 0, value};
}

SsaValue MakeVar(uint32_t id) {
    return SsaValue{SsaValueKind::Variable, id, 0};
}

SsaValue MakeTemp(uint32_t id) {
    return SsaValue{SsaValueKind::Temporary, id, 0};
}

class BuildContext {
public:
    SsaProgram program{};

    SsaValue BuildValue(const AST::Expr* node) {
        if (node == nullptr)
            return MakeInvalid();

        if (node->op == AST::OpType::Const)
            return MakeConst(node->constValue);

        if (node->op == AST::OpType::Var)
            return MakeVar(node->id.value());

        const auto it = tempByNode.find(node);
        if (it != tempByNode.end())
            return MakeTemp(it->second);

        std::vector<SsaValue> inputValues;
        inputValues.reserve(node->inputs.size());
        for (const AST::Expr* input : node->inputs)
            inputValues.push_back(BuildValue(input));

        const uint32_t tempId = nextTempId++;
        tempByNode.emplace(node, tempId);
        program.instructions.push_back(SsaInstruction{tempId, node->op, std::move(inputValues)});
        return MakeTemp(tempId);
    }

private:
    uint32_t nextTempId = 0;
    std::unordered_map<const AST::Expr*, uint32_t> tempByNode{};
};

} // namespace

SsaProgram SsaBuilder::Build(const AST::Expr* root) {
    return Build(std::vector<const AST::Expr*>{root});
}

SsaProgram SsaBuilder::Build(const std::vector<const AST::Expr*>& roots) {
    BuildContext context{};
    context.program.outputs.reserve(roots.size());

    for (const AST::Expr* root : roots)
        context.program.outputs.push_back(context.BuildValue(root));

    return context.program;
}

} // namespace BitFlow::Core::Codegen::SSA
