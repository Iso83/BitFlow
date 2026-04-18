#include "codegen/PerfPass.h"

#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <TestAssert.h>
#include <cstdint>
#include <unordered_map>
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
        {2u, static_cast<uint32_t>(AST::OpType::Add), {0u, 1u}},             // root
        {3u, static_cast<uint32_t>(AST::OpType::Mul), {VarId(3), VarId(4)}}  // unused
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

    // CSE exact match only: (a ^ b) != (b ^ a)
    {
        std::vector<Codegen::Statement> exactOnly = {
            {0u, static_cast<uint32_t>(AST::OpType::Xor), {VarId(7), VarId(8)}},
            {1u, static_cast<uint32_t>(AST::OpType::Xor), {VarId(8), VarId(7)}},
            {2u, static_cast<uint32_t>(AST::OpType::Add), {0u, 1u}}};

        Codegen::ApplyPerfPass(exactOnly, 2u);
        BF_TEST(exactOnly.size() == 3u); // geen CSE op niet-exacte input-volgorde
    }

    // Stap 20.3 — constant tracking via constValues map (statement-level fold).
    {
        constexpr uint32_t kConstTag = 0x80000000u;
        uint32_t rootId = 1u;
        std::unordered_map<uint32_t, uint64_t> constValues = {
            {kConstTag | 1u, 250u},
            {kConstTag | 2u, 10u},
        };

        std::vector<Codegen::Statement> foldable = {
            {0u, static_cast<uint32_t>(AST::OpType::Add), {kConstTag | 1u, kConstTag | 2u}},
            {1u, static_cast<uint32_t>(AST::OpType::Xor), {0u, kConstTag | 2u}},
        };

        Codegen::ApplyPerfPass(foldable, rootId, constValues, 8u);
        BF_TEST(foldable.empty());
        BF_TEST((rootId & kConstTag) != 0u);
        BF_TEST(constValues.count(rootId) == 1u);
        BF_TEST(constValues[rootId] == 14u); // ((250+10)&0xFF) ^ 10 = 14
    }

    // Stap 20.7 — shifts modulo bitWidth (no UB on large shift amounts).
    {
        constexpr uint32_t kConstTag = 0x80000000u;
        std::vector<Codegen::Statement> shiftFold = {
            {0u, static_cast<uint32_t>(AST::OpType::Shl), {kConstTag | 3u, kConstTag | 130u}},
        };

        Codegen::ApplyConstantFolding(shiftFold, 8u);
        BF_TEST(shiftFold.size() == 1u);
        BF_TEST(shiftFold[0].inputs.empty()); // folded to leaf-like statement
    }

    return 0;
}
