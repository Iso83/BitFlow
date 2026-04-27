#include <BitFlow/core/codegen_ssa/SsaBuilder.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <string>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Testing;

int main() {
    using Codegen::BuildSSA;

    // Null root should return an empty SSA program.
    {
        auto p = BuildSSA(nullptr, 32);
        BF_TEST(p.statements.empty());
        BF_TEST(p.result.empty());
        BF_TEST(p.results.empty());
    }

    // Leaf-only tree should stay inline (no non-leaf => no SSA statements).
    {
        auto v1 = MakeVar(1);
        auto p = BuildSSA(v1, 32);
        BF_TEST(p.statements.empty());
        BF_TEST(!p.result.empty());
        BF_TEST(p.results.size() == 1u);
        BF_TEST(p.results[0] == p.result);
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
        BF_TEST(!p.statements[1].name.empty());
        BF_TEST(!p.result.empty());
        BF_TEST(p.results.size() == 1u);
        BF_TEST(p.results[0] == p.result);
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
        BF_TEST(!prog.result.empty());
        BF_TEST(prog.results.size() == 1u);
        BF_TEST(prog.results[0] == prog.result);
    }

    // Stap 18.2 — post-pass: CSE + DCE + temp-compactie op scheduled SSA.
    {
        auto a = MakeVar(21);
        auto b = MakeVar(22);
        auto x1 = MakeOp(210, OpType::Xor, {a, b});
        auto x2 = MakeOp(211, OpType::Xor, {a, b}); // niet pointer-gedeeld
        auto expr = MakeOp(212, OpType::Add, {x1, x2});

        auto prog = BuildSSA(expr, 32);
        BF_TEST(prog.statements.size() == 2u); // xor + add
        BF_TEST(prog.statements[0].name == "t0");
        BF_TEST(!prog.statements[1].name.empty());
        BF_TEST(prog.statements[1].expr.find("t0") != std::string::npos);
        BF_TEST(!prog.result.empty());
        BF_TEST(prog.results.size() == 1u);
        BF_TEST(prog.results[0] == prog.result);
    }

    // Stap 20.2 — statement-level constant folding for pure ops (all-const inputs only).
    {
        auto c250 = MakeConst(302, 250);
        auto c10 = MakeConst(303, 10);
        auto add = MakeOp(300, OpType::Add, {c250, c10}); // 260 -> 4 on 8-bit

        auto prog = BuildSSA(add, 8);
        BF_TEST(prog.statements.empty());
        BF_TEST(prog.result == "0x4u");
    }

    // Stap 20.2 — do not fold when not all inputs are constant.
    {
        auto v1 = MakeVar(1);
        auto c1 = MakeConst(304, 1);
        auto add = MakeOp(301, OpType::Add, {v1, c1});

        auto prog = BuildSSA(add, 8);
        BF_TEST(prog.statements.size() == 1u);
        BF_TEST(prog.result == "t0");
    }

    // Stap 20.8 — ((2 + 3) * 4) => 20, folded to direct constant root.
    {
        auto c2 = MakeConst(401, 2);
        auto c3 = MakeConst(402, 3);
        auto c4 = MakeConst(403, 4);
        auto add = MakeOp(404, OpType::Add, {c2, c3});
        auto mul = MakeOp(405, OpType::Mul, {add, c4});

        auto prog = BuildSSA(mul, 8);
        BF_TEST(prog.statements.empty()); // aantal statements omlaag naar direct constant root
        BF_TEST(prog.result == "0x14u");
    }

    return 0;
}
