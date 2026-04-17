#pragma once

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>
#include <cstdint>
#include <vector>

namespace BitFlow::Core::Codegen::SSA {

enum class SsaValueKind {
    Invalid,
    Temporary,
    Variable,
    Constant,
};

struct SsaValue {
    SsaValueKind kind = SsaValueKind::Invalid;
    uint32_t id = 0;
    uint64_t constant = 0;
};

struct SsaInstruction {
    uint32_t resultId = 0;
    AST::OpType op = AST::OpType::Const;
    std::vector<SsaValue> inputs{};
};

struct SsaProgram {
    std::vector<SsaInstruction> instructions{};
    std::vector<SsaValue> outputs{};
};

class SsaBuilder {
public:
    SsaProgram Build(const AST::Expr* root);
    SsaProgram Build(const std::vector<const AST::Expr*>& roots);
};

} // namespace BitFlow::Core::Codegen::SSA
