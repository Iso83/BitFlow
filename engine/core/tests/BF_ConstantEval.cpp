#include <BitFlow/core/eval/ConstantEval.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Eval;

int TestEvaluate_PureConstantExpression() {
    auto c = MakeConst(1, 0xABCD1234);

    EvalResult r = EvaluateConstant(c, 32);

    BF_TEST(r.status == EvalStatus::Success);
    BF_TEST(r.value == 0xABCD1234ULL);
    return 0;
}

int TestEvaluate_NestedExpression() {
    auto c10 = MakeConst(1, 10);
    auto c20 = MakeConst(2, 20);
    auto c7 = MakeConst(3, 7);
    auto c3 = MakeConst(4, 3);
    auto c55 = MakeConst(5, 0x55);

    auto add = MakeOp(6, OpType::Add, {c10, c20});
    auto sub = MakeOp(7, OpType::Sub, {c7, c3});
    auto mul = MakeOp(8, OpType::Mul, {add, sub});
    auto expr = MakeOp(9, OpType::Xor, {mul, c55});

    EvalResult r = EvaluateConstant(expr, 16);

    BF_TEST(r.status == EvalStatus::Success);
    BF_TEST(r.value == 0x2DULL);
    return 0;
}

int TestEvaluate_BitWidthVariations_8_16_32_64() {
    auto a = MakeConst(1, 0xFFFF0000);
    auto b = MakeConst(2, 0x1234);
    auto expr = MakeOp(3, OpType::Add, {a, b});

    EvalResult r8 = EvaluateConstant(expr, 8);
    EvalResult r16 = EvaluateConstant(expr, 16);
    EvalResult r32 = EvaluateConstant(expr, 32);
    EvalResult r64 = EvaluateConstant(expr, 64);

    BF_TEST(r8.status == EvalStatus::Success);
    BF_TEST(r8.value == 0x34ULL);

    BF_TEST(r16.status == EvalStatus::Success);
    BF_TEST(r16.value == 0x1234ULL);

    BF_TEST(r32.status == EvalStatus::Success);
    BF_TEST(r32.value == 0xFFFF1234ULL);

    BF_TEST(r64.status == EvalStatus::Success);
    BF_TEST(r64.value == 0xFFFF1234ULL);
    return 0;
}

int TestEvaluate_ShiftRotateEdgeCases() {
    auto v = MakeConst(1, 0x81);
    auto z = MakeConst(2, 0);
    auto w = MakeConst(3, 8);
    auto n = MakeConst(4, 9);

    auto shl0 = MakeOp(5, OpType::Shl, {v, z});
    auto shr0 = MakeOp(6, OpType::Shr, {v, z});
    auto rotl0 = MakeOp(7, OpType::RotL, {v, z});
    auto rotr0 = MakeOp(8, OpType::RotR, {v, z});

    auto shlW = MakeOp(9, OpType::Shl, {v, w});
    auto shrW = MakeOp(10, OpType::Shr, {v, w});
    auto rotlW = MakeOp(11, OpType::RotL, {v, w});
    auto rotrW = MakeOp(12, OpType::RotR, {v, w});

    auto shlN = MakeOp(13, OpType::Shl, {v, n});
    auto shrN = MakeOp(14, OpType::Shr, {v, n});
    auto ushrN = MakeOp(15, OpType::UShr, {v, n});
    auto rotlN = MakeOp(16, OpType::RotL, {v, n});
    auto rotrN = MakeOp(17, OpType::RotR, {v, n});

    BF_TEST(EvaluateConstant(shl0, 8).value == 0x81ULL);
    BF_TEST(EvaluateConstant(shr0, 8).value == 0x81ULL);
    BF_TEST(EvaluateConstant(rotl0, 8).value == 0x81ULL);
    BF_TEST(EvaluateConstant(rotr0, 8).value == 0x81ULL);

    BF_TEST(EvaluateConstant(shlW, 8).value == 0x81ULL);
    BF_TEST(EvaluateConstant(shrW, 8).value == 0x81ULL);
    BF_TEST(EvaluateConstant(rotlW, 8).value == 0x81ULL);
    BF_TEST(EvaluateConstant(rotrW, 8).value == 0x81ULL);

    BF_TEST(EvaluateConstant(shlN, 8).value == 0x02ULL);
    BF_TEST(EvaluateConstant(shrN, 8).value == 0x40ULL);
    BF_TEST(EvaluateConstant(ushrN, 8).value == 0x40ULL);
    BF_TEST(EvaluateConstant(rotlN, 8).value == 0x03ULL);
    BF_TEST(EvaluateConstant(rotrN, 8).value == 0xC0ULL);
    return 0;
}

int TestEvaluate_DivisionModuloByZero() {
    auto a = MakeConst(1, 7);
    auto z = MakeConst(2, 0);

    auto div = MakeOp(3, OpType::Div, {a, z});
    auto mod = MakeOp(4, OpType::Mod, {a, z});

    EvalResult rDiv = EvaluateConstant(div, 32);
    EvalResult rMod = EvaluateConstant(mod, 32);

    BF_TEST(rDiv.status == EvalStatus::DivisionByZero);
    BF_TEST(rMod.status == EvalStatus::ModuloByZero);
    return 0;
}

int TestEvaluate_NotConstantCases() {
    auto x = MakeVar(1);
    auto c1 = MakeConst(2, 5);
    auto c2 = MakeConst(3, 6);

    auto mixed = MakeOp(4, OpType::Add, {x, c1});
    auto nested = MakeOp(5, OpType::Mul, {mixed, c2});

    EvalResult rLeaf = EvaluateConstant(x, 32);
    EvalResult rMixed = EvaluateConstant(mixed, 32);
    EvalResult rNested = EvaluateConstant(nested, 32);

    BF_TEST(rLeaf.status == EvalStatus::NotConstant);
    BF_TEST(rMixed.status == EvalStatus::NotConstant);
    BF_TEST(rNested.status == EvalStatus::NotConstant);
    return 0;
}

int TestEvaluate_InvalidBitWidth() {
    auto c = MakeConst(1, 5);

    EvalResult r0 = EvaluateConstant(c, 0);

    BF_TEST(r0.status == EvalStatus::InvalidBitWidth);
    BF_TEST(EvaluateConstant(c, 65).status == EvalStatus::Success);
    return 0;
}

int TestEvaluate_WideBitWidth_UsesBfUintPath() {
    auto c1 = MakeConst(1, 0x81);
    auto c2 = MakeConst(2, 1);

    auto rotl = MakeOp(3, OpType::RotL, {c1, c2});
    auto shl = MakeOp(4, OpType::Shl, {c1, c2});
    auto add = MakeOp(5, OpType::Add, {rotl, shl});
    auto mod = MakeOp(6, OpType::Mod, {add, MakeConst(7, 257)});

    auto wide = EvaluateConstantWide(mod, 128);
    EvalResult r = EvaluateConstant(mod, 128);

    BF_TEST(wide.status == EvalStatus::Success);
    BF_TEST(wide.value.BitWidth() == 128U);
    BF_TEST(r.status == EvalStatus::Success);
    BF_TEST(r.value == ((0x102ULL + 0x102ULL) % 257ULL));

    auto rotr = MakeOp(8, OpType::RotR, {c2, MakeConst(9, 1)});
    auto rotrWide = EvaluateConstantWide(rotr, 128);
    BF_TEST(rotrWide.status == EvalStatus::Success);
    BF_TEST(rotrWide.value.Shr(127).ToUint64() == 1ULL);
    return 0;
}

int main() {
    BF_RUN_TEST(TestEvaluate_PureConstantExpression);
    BF_RUN_TEST(TestEvaluate_NestedExpression);
    BF_RUN_TEST(TestEvaluate_BitWidthVariations_8_16_32_64);
    BF_RUN_TEST(TestEvaluate_ShiftRotateEdgeCases);
    BF_RUN_TEST(TestEvaluate_DivisionModuloByZero);
    BF_RUN_TEST(TestEvaluate_NotConstantCases);
    BF_RUN_TEST(TestEvaluate_InvalidBitWidth);
    BF_RUN_TEST(TestEvaluate_WideBitWidth_UsesBfUintPath);
    return 0;
}
