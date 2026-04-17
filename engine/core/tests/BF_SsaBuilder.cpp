#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <vector>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Codegen::SSA;

int main() {
    SsaBuilder builder;

    // Null root should produce an invalid output value and no instructions.
    {
        SsaProgram p = builder.Build(static_cast<const AST::Expr*>(nullptr));
        BF_TEST(p.instructions.empty());
        BF_TEST(p.outputs.size() == 1u);
        BF_TEST(p.outputs[0].kind == SsaValueKind::Invalid);
    }

    // Leaf-only tree should not create temporaries.
    {
        auto v1 = MakeVar(1);
        SsaProgram p = builder.Build(v1);
        BF_TEST(p.instructions.empty());
        BF_TEST(p.outputs.size() == 1u);
        BF_TEST(p.outputs[0].kind == SsaValueKind::Variable);
        BF_TEST(p.outputs[0].id == 1u);
    }

    // Shared sub-tree should be emitted once.
    {
        auto a = MakeVar(10);
        auto b = MakeVar(11);
        auto c = MakeVar(12);

        auto x = MakeOp(100, OpType::Xor, {a, b});
        auto out1 = MakeOp(101, OpType::Add, {x, c});
        auto out2 = MakeOp(102, OpType::And, {x, c});

        SsaProgram p = builder.Build(std::vector<const AST::Expr*>{out1, out2});

        BF_TEST(p.instructions.size() == 3u); // xor, add, and
        BF_TEST(p.outputs.size() == 2u);
        BF_TEST(p.outputs[0].kind == SsaValueKind::Temporary);
        BF_TEST(p.outputs[1].kind == SsaValueKind::Temporary);

        BF_TEST(p.instructions[0].op == OpType::Xor);
        BF_TEST(p.instructions[1].op == OpType::Add);
        BF_TEST(p.instructions[2].op == OpType::And);
    }

    return 0;
}
