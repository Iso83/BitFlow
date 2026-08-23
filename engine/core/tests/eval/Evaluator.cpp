#include "TestAssert.h"
#include "common/Expr.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Eval;
using namespace BitFlow::Engine::Core::Expression;

#pragma region EvaluateConstant
int TestEvaluate_PureConstantExpression() {
    MakeExprStore(32);

    EvalResult r = EvaluateConstant(C(0xABCD1234), bitWidth);

    CPPTEST_ASSERT(r.status == EvalStatus::Success);
    CPPTEST_ASSERT(EqualChunkValue(r.value, 0xABCD1234ULL));
    return 0;
}

int TestEvaluate_NestedExpression() {
    MakeExprStore(32);

    auto add = C(10) + C(20);
    auto sub = C(7) - C(3);
    auto mul = add * sub;
    auto expr = mul ^ C(0x55);

    EvalResult r = EvaluateConstant(expr, 16);

    CPPTEST_ASSERT(r.status == EvalStatus::Success);
    CPPTEST_ASSERT(EqualChunkValue(r.value, 0x2DULL));
    return 0;
}

int TestEvaluate_BitWidthVariations_8_16_32_64() {
    MakeExprStore(32);

    auto a = C(0xFFFF0000);
    auto b = C(0x1234);
    auto expr = a + b;

    EvalResult r8 = EvaluateConstant(expr, 8);
    EvalResult r16 = EvaluateConstant(expr, 16);
    EvalResult r32 = EvaluateConstant(expr, 32);
    EvalResult r64 = EvaluateConstant(expr, 64);

    CPPTEST_ASSERT(r8.status == EvalStatus::Success);
    CPPTEST_ASSERT(EqualChunkValue(r8.value, 0x34ULL));

    CPPTEST_ASSERT(r16.status == EvalStatus::Success);
    CPPTEST_ASSERT(EqualChunkValue(r16.value, 0x1234ULL));

    CPPTEST_ASSERT(r32.status == EvalStatus::Success);
    CPPTEST_ASSERT(EqualChunkValue(r32.value, 0xFFFF1234ULL));

    CPPTEST_ASSERT(r64.status == EvalStatus::Success);
    CPPTEST_ASSERT(EqualChunkValue(r64.value, 0xFFFF1234ULL));
    return 0;
}

int TestPowConst() {
    MakeExprStore(32);

    auto expr = C(2).Pow(C(8));

    auto r = EvaluateConstant(expr, bitWidth);

    CPPTEST_ASSERT(r.status == EvalStatus::Success);
    CPPTEST_ASSERT(EqualChunkValue(r.value, 256));
    return 0;
}

int TestPowZeroExponent() {
    MakeExprStore(32);

    auto expr = C(7).Pow(C(0));

    auto r = EvaluateConstant(expr, bitWidth);

    CPPTEST_ASSERT(r.status == EvalStatus::Success);
    CPPTEST_ASSERT(EqualChunkValue(r.value, 1));
    return 0;
}

int TestPowOneExponent() {
    MakeExprStore(32);

    auto expr = C(9).Pow(C(1));

    auto r = EvaluateConstant(expr, bitWidth);

    CPPTEST_ASSERT(r.status == EvalStatus::Success);
    CPPTEST_ASSERT(EqualChunkValue(r.value, 9));
    return 0;
}

int TestPowZeroBase() {
    MakeExprStore(32);

    auto expr = C(0).Pow(C(5));

    auto r = EvaluateConstant(expr, bitWidth);

    CPPTEST_ASSERT(r.status == EvalStatus::Success);
    CPPTEST_ASSERT(EqualChunkValue(r.value, 0));
    return 0;
}

int TestPowBitWidthMasking() {
    MakeExprStore(8);

    auto expr = C(2).Pow(C(8));

    auto r = EvaluateConstant(expr, bitWidth);

    CPPTEST_ASSERT(r.status == EvalStatus::Success);
    CPPTEST_ASSERT(EqualChunkValue(r.value, 0));
    return 0;
}

int TestPowNestedExpression() {
    MakeExprStore(32);

    auto expr = (C(2) + C(1)).Pow(C(3));

    auto r = EvaluateConstant(expr, bitWidth);

    CPPTEST_ASSERT(r.status == EvalStatus::Success);
    CPPTEST_ASSERT(EqualChunkValue(r.value, 27));
    return 0;
}

int TestEvaluate_ShiftRotateEdgeCases() {
    MakeExprStore(32);

    auto v = C(0x81);

    auto z = C(0);
    CPPTEST_ASSERT(Eval(v << z, 0x81ULL));
    CPPTEST_ASSERT(Eval(v >> z, 0x81ULL));
    CPPTEST_ASSERT(Eval(v.RotL(z), 0x81ULL));
    CPPTEST_ASSERT(Eval(v.RotR(z), 0x81ULL));

    auto w = C(8);
    CPPTEST_ASSERT(Eval(v << w, 0x8100ULL));
    CPPTEST_ASSERT(Eval(v >> w, 0x0ULL));
    CPPTEST_ASSERT(Eval(v.RotL(w), 0x8100ULL));
    CPPTEST_ASSERT(Eval(v.RotR(w), 0x81000000ULL));

    auto n = C(9);
    CPPTEST_ASSERT(Eval(v << n, 0x10200ULL));
    CPPTEST_ASSERT(Eval(v >> n, 0x0ULL));
    CPPTEST_ASSERT(Eval(v.RotL(n), 0x10200ULL));
    CPPTEST_ASSERT(Eval(v.RotR(n), 0x40800000ULL));
    return 0;
}

int TestEvaluate_DivisionModuloByZero() {
    MakeExprStore(32);

    auto a = C(7);
    auto z = C(0);

    auto div = a / z;
    auto mod = a % z;

    EvalResult rDiv = EvaluateConstant(div, 32);
    EvalResult rMod = EvaluateConstant(mod, 32);

    CPPTEST_ASSERT(rDiv.status == EvalStatus::DivisionByZero);
    CPPTEST_ASSERT(rMod.status == EvalStatus::ModuloByZero);
    return 0;
}

int TestEvaluate_NotConstantCases() {
    MakeExprStore(32);

    auto x = V();
    auto c1 = C(5);
    auto c2 = C(6);

    auto mixed = x + c1;
    auto nested = mixed * c2;

    EvalResult rLeaf = EvaluateConstant(x, 32);
    EvalResult rMixed = EvaluateConstant(mixed, 32);
    EvalResult rNested = EvaluateConstant(nested, 32);

    CPPTEST_ASSERT(rLeaf.status == EvalStatus::NotConstant);
    CPPTEST_ASSERT(rMixed.status == EvalStatus::NotConstant);
    CPPTEST_ASSERT(rNested.status == EvalStatus::NotConstant);
    return 0;
}

int TestEvaluate_InvalidBitWidth() {
    MakeExprStore(32);

    auto c = C(5);

    EvalResult r0 = EvaluateConstant(c, 0);

    CPPTEST_ASSERT(r0.status == EvalStatus::InvalidBitWidth);
    CPPTEST_ASSERT(EvaluateConstant(c, 65).status == EvalStatus::Success);
    return 0;
}

int TestEvaluate_WideBitWidth_UsesBfUintPath() {
    MakeExprStore(32);

    auto c1 = C(0x81);
    auto c2 = C(1);

    auto rotl = c1.RotL(c2);
    auto shl = c1 << c2;
    auto add = rotl + shl;
    auto mod = add % C(257);

    EvalResult r = EvaluateConstant(mod, 128);

    CPPTEST_ASSERT(r.status == EvalStatus::Success);
    CPPTEST_ASSERT(r.value.BitWidth() == 128U);
    CPPTEST_ASSERT(EqualChunkValue(r.value, ((0x102ULL + 0x102ULL) % 257ULL)));

    auto rotr = c2.RotR(1);
    auto rotrWide = EvaluateConstant(rotr, 128);
    CPPTEST_ASSERT(rotrWide.status == EvalStatus::Success);
    CPPTEST_ASSERT(rotrWide.value.Shr(127).ToChunk() == 1ULL);
    return 0;
}
#pragma endregion

#pragma region IsFullyConstant
int TestDetect_ConstLeaf() {
    MakeExprStore(32);
    CPPTEST_ASSERT(IsFullyConstant(C(42)));
    return 0;
}

int TestDetect_VarLeafFalse() {
    MakeExprStore(32);
    CPPTEST_ASSERT(!IsFullyConstant(V()));
    return 0;
}

int TestDetect_AllChildrenConstant() {
    MakeExprStore(32);
    auto expr = C(10) + C(20);

    CPPTEST_ASSERT(IsFullyConstant(expr));
    return 0;
}

int TestDetect_AnyChildNonConstantFalse() {
    MakeExprStore(32);
    auto expr = C(10) + V();

    CPPTEST_ASSERT(!IsFullyConstant(expr));
    return 0;
}
#pragma endregion

int main() {
    CPPTEST_RUN(TestEvaluate_PureConstantExpression);
    CPPTEST_RUN(TestEvaluate_NestedExpression);
    CPPTEST_RUN(TestEvaluate_BitWidthVariations_8_16_32_64);

    CPPTEST_RUN(TestPowConst);
    CPPTEST_RUN(TestPowZeroExponent);
    CPPTEST_RUN(TestPowOneExponent);
    CPPTEST_RUN(TestPowZeroBase);
    CPPTEST_RUN(TestPowBitWidthMasking);
    CPPTEST_RUN(TestPowNestedExpression);
    CPPTEST_RUN(TestPowNestedExpression);

    CPPTEST_RUN(TestEvaluate_ShiftRotateEdgeCases);
    CPPTEST_RUN(TestEvaluate_DivisionModuloByZero);
    CPPTEST_RUN(TestEvaluate_NotConstantCases);
    CPPTEST_RUN(TestEvaluate_InvalidBitWidth);
    CPPTEST_RUN(TestEvaluate_WideBitWidth_UsesBfUintPath);

    CPPTEST_RUN(TestDetect_ConstLeaf);
    CPPTEST_RUN(TestDetect_VarLeafFalse);
    CPPTEST_RUN(TestDetect_AllChildrenConstant);
    CPPTEST_RUN(TestDetect_AnyChildNonConstantFalse);
    return 0;
}
