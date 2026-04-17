#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <string>

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

    // Leaf-only tree should stay inline (no non-leaf => no SSA statements).
    {
        auto v1 = MakeVar(1);
        auto p = BuildSSA(v1, 32);
        BF_TEST(p.statements.empty());
        BF_TEST(!p.result.empty());
    }

    // Shared non-leaf sub-tree should be emitted once, in post-order.
    {
        auto a = MakeVar(10);
        auto b = MakeVar(11);
        auto c = MakeVar(12);

        auto x = MakeOp(100, OpType::Xor, {a, b});
        auto out = MakeOp(101, OpType::Add, {x, x, c});

        auto p = BuildSSA(out, 32);
        BF_TEST(p.statements.size() == 2u); // xor, add
        BF_TEST(p.statements[0].name == "t0");
        BF_TEST(p.statements[1].name == "t1");
        BF_TEST(p.result == "t1");
        BF_TEST(p.statements[1].expr.find("t0") != std::string::npos);
    }

    // Stap 14.6 — exact regression case
    {
        auto a = MakeVar(1);
        auto b = MakeVar(2);
        auto add = MakeOp(10, OpType::Add, {a, b});
        auto expr = MakeOp(11, OpType::Mul, {add, add});

        auto prog = BuildSSA(expr, 32);
        BF_TEST(prog.statements.size() == 2);
        BF_TEST(prog.result == "t1");
    }

    return 0;
}
