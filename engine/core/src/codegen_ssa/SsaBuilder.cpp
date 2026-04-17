#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace BitFlow::Core::Codegen {
namespace {

std::string MaskExpr(uint32_t bitWidth) {
    if (bitWidth >= 64)
        return "0xffffffffffffffffull";

    return "((1ull << " + std::to_string(bitWidth) + ") - 1ull)";
}

std::string ApplyMask(const std::string& expr, uint32_t bitWidth) {
    return "((" + expr + ") & " + MaskExpr(bitWidth) + ")";
}

std::string NormalizeShift(const std::string& shiftExpr, uint32_t bitWidth) {
    return "((" + shiftExpr + ") % " + std::to_string(bitWidth) + "ull)";
}

std::string EmitBinary(const std::string& lhs, const char* op, const std::string& rhs) {
    return "(" + lhs + " " + std::string(op) + " " + rhs + ")";
}

std::string EmitNary(const std::vector<std::string>& inputs, const char* op) {
    if (inputs.empty())
        return "0ull";

    std::string out = inputs.front();
    for (size_t i = 1; i < inputs.size(); ++i)
        out = EmitBinary(out, op, inputs[i]);

    return out;
}

std::string EmitRotate(const std::string& value, const std::string& shift, uint32_t bitWidth, bool left) {
    const std::string bw = std::to_string(bitWidth) + "ull";
    if (left)
        return ApplyMask("((" + value + " << " + shift + ") | (" + value + " >> (" + bw + " - " + shift + ")))", bitWidth);

    return ApplyMask("((" + value + " >> " + shift + ") | (" + value + " << (" + bw + " - " + shift + ")))", bitWidth);
}

std::string EmitOp(AST::OpType op, const std::vector<std::string>& inputs, uint32_t bitWidth) {
    if (inputs.empty())
        return "0ull";

    switch (op) {
    case AST::OpType::Not:
        if (inputs.size() != 1)
            return "0ull";
        return ApplyMask("(~" + inputs[0] + ")", bitWidth);

    case AST::OpType::Neg:
        if (inputs.size() != 1)
            return "0ull";
        return ApplyMask("(-" + inputs[0] + ")", bitWidth);

    case AST::OpType::And:
        return ApplyMask(EmitNary(inputs, "&"), bitWidth);
    case AST::OpType::Or:
        return ApplyMask(EmitNary(inputs, "|"), bitWidth);
    case AST::OpType::Xor:
        return ApplyMask(EmitNary(inputs, "^"), bitWidth);
    case AST::OpType::Add:
        return ApplyMask(EmitNary(inputs, "+"), bitWidth);
    case AST::OpType::Mul:
        return ApplyMask(EmitNary(inputs, "*"), bitWidth);

    case AST::OpType::Sub:
    case AST::OpType::Div:
    case AST::OpType::Mod:
    case AST::OpType::Shl:
    case AST::OpType::Shr:
    case AST::OpType::UShr:
    case AST::OpType::RotL:
    case AST::OpType::RotR:
        if (inputs.size() != 2)
            return "0ull";
        break;

    case AST::OpType::Ch:
    case AST::OpType::Maj:
        if (inputs.size() != 3)
            return "0ull";
        break;

    case AST::OpType::Var:
    case AST::OpType::Const:
    default:
        return "0ull";
    }

    switch (op) {
    case AST::OpType::Sub:
        return ApplyMask(EmitBinary(inputs[0], "-", inputs[1]), bitWidth);
    case AST::OpType::Div:
        return ApplyMask(EmitBinary(inputs[0], "/", inputs[1]), bitWidth);
    case AST::OpType::Mod:
        return ApplyMask(EmitBinary(inputs[0], "%", inputs[1]), bitWidth);
    case AST::OpType::Shl:
        return ApplyMask(EmitBinary(inputs[0], "<<", NormalizeShift(inputs[1], bitWidth)), bitWidth);
    case AST::OpType::Shr:
    case AST::OpType::UShr:
        return ApplyMask(EmitBinary(inputs[0], ">>", NormalizeShift(inputs[1], bitWidth)), bitWidth);
    case AST::OpType::RotL:
        return EmitRotate(inputs[0], NormalizeShift(inputs[1], bitWidth), bitWidth, true);
    case AST::OpType::RotR:
        return EmitRotate(inputs[0], NormalizeShift(inputs[1], bitWidth), bitWidth, false);
    case AST::OpType::Ch:
        return ApplyMask("((" + inputs[0] + " & " + inputs[1] + ") ^ ((~" + inputs[0] + ") & " + inputs[2] + "))", bitWidth);
    case AST::OpType::Maj:
        return ApplyMask("((" + inputs[0] + " & " + inputs[1] + ") ^ (" + inputs[0] + " & " + inputs[2] + ") ^ (" + inputs[1] + " & " + inputs[2] + "))", bitWidth);
    default:
        return "0ull";
    }
}

struct BuildContext {
    uint32_t bitWidth = 64;
    uint32_t nextTemp = 0;
    SsaProgram program{};
    std::unordered_map<const AST::Expr*, std::string> tempByNode{};

    std::string Lower(const AST::Expr* node) {
        if (node == nullptr)
            return "0ull";

        if (node->op == AST::OpType::Const)
            return ApplyMask(std::to_string(node->constValue) + "ull", bitWidth);

        if (node->op == AST::OpType::Var)
            return ApplyMask("v" + std::to_string(node->id.value()), bitWidth);

        auto it = tempByNode.find(node);
        if (it != tempByNode.end())
            return it->second;

        std::vector<std::string> loweredInputs;
        loweredInputs.reserve(node->inputs.size());
        for (const AST::Expr* input : node->inputs)
            loweredInputs.push_back(Lower(input));

        const std::string name = "t" + std::to_string(nextTemp++);
        const std::string expr = EmitOp(node->op, loweredInputs, bitWidth);
        program.statements.push_back(SsaStatement{name, expr});
        tempByNode.emplace(node, name);
        return name;
    }
};

} // namespace

SsaProgram BuildSSA(const AST::Expr* root, uint32_t bitWidth) {
    SsaProgram program{};
    if (root == nullptr || bitWidth == 0 || bitWidth > 64)
        return program;

    BuildContext context{};
    context.bitWidth = bitWidth;
    context.program.result = context.Lower(root);
    return context.program;
}

} // namespace BitFlow::Core::Codegen
