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

    // (a + b) * (a + b) => t0=(v1+v2), t1=(t0*t0), return t1
    {
        auto a = MakeVar(1);
        auto b = MakeVar(2);
        auto sum = MakeOp(200, OpType::Add, {a, b});
        auto mul = MakeOp(201, OpType::Mul, {sum, sum});

        auto p = BuildSSA(mul, 32);
        BF_TEST(p.statements.size() == 2u);
        BF_TEST(p.statements[0].name == "t0");
        BF_TEST(p.statements[0].expr.find("v1") != std::string::npos);
        BF_TEST(p.statements[0].expr.find("v2") != std::string::npos);
        BF_TEST(p.statements[1].name == "t1");
        BF_TEST(p.statements[1].expr.find("t0") != std::string::npos);
        BF_TEST(p.result == "t1");
    }

    return 0;
}
