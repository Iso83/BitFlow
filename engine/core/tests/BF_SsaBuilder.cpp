#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Testing;

int main() {
    using Codegen::BuildSSA;

    // Null root should return an empty SSA program.
    {
        auto p = BuildSSA(nullptr, 32);
        BF_TEST(p.statements.empty());
        BF_TEST(p.result.empty());
    }

    // Leaf-only tree should still allocate exactly one SSA variable.
    {
        auto v1 = MakeVar(1);
        auto p = BuildSSA(v1, 32);
        BF_TEST(p.statements.size() == 1u);
        BF_TEST(p.statements[0].name == "t0");
        BF_TEST(p.result == "t0");
    }

    // Shared sub-tree should be emitted exactly once, with one SSA var per unique node.
    {
        auto a = MakeVar(10);
        auto b = MakeVar(11);
        auto c = MakeVar(12);

        auto x = MakeOp(100, OpType::Xor, {a, b});
        auto out = MakeOp(101, OpType::Add, {x, x, c});

        auto p = BuildSSA(out, 32);
        BF_TEST(p.statements.size() == 5u); // a, b, xor, c, add
        BF_TEST(p.statements[0].name == "t0");
        BF_TEST(p.statements[4].name == "t4");
        BF_TEST(p.result == "t4");
    }

    return 0;
}
