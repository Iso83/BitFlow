#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/codegen_ssa/SsaBuilder.h>

#include "codegen/PerfPass.h"
#include <TestAssert.h>

#include <cstdint>
#include <vector>

using namespace BitFlow::Core;

namespace {

uint32_t VarId(uint32_t id) {
    return 0x40000000u | id;
}

const Codegen::Statement* FindById(const std::vector<Codegen::Statement>& stmts, uint32_t id) {
    for (const auto& s : stmts)
        if (s.id == id)
            return &s;
    return nullptr;
}

} // namespace

int main() {
    std::vector<Codegen::Statement> stmts = {
        {0u, static_cast<uint32_t>(AST::OpType::Xor), {VarId(1), VarId(2)}},
        {1u, static_cast<uint32_t>(AST::OpType::Xor), {VarId(1), VarId(2)}}, // duplicate subtree
        {2u, static_cast<uint32_t>(AST::OpType::Add), {0u, 1u}},            // root
        {3u, static_cast<uint32_t>(AST::OpType::Mul), {VarId(3), VarId(4)}} // unused
    };

    const size_t beforeCount = stmts.size();
    Codegen::ApplyPerfPass(stmts, 2u);

    // identieke subtrees -> 1 compute
    BF_TEST(FindById(stmts, 1u) == nullptr);
    const Codegen::Statement* root = nullptr;
    for (const auto& st : stmts)
        if (st.op == static_cast<uint32_t>(AST::OpType::Add))
            root = &st;

    BF_TEST(root != nullptr);
    BF_TEST(root->inputs.size() == 2u);
    BF_TEST(root->inputs[0] == 0u);
    BF_TEST(root->inputs[1] == 0u);

    // unused nodes -> verdwijnen
    BF_TEST(FindById(stmts, 3u) == nullptr);

    // aantal temps omlaag
    BF_TEST(stmts.size() < beforeCount);
    BF_TEST(stmts.size() == 2u);

    return 0;
}
