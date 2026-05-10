#include <BitFlow/core/eval/Evaluator.h>
#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Eval;
using namespace BitFlow::Core::Expression;

#pragma region EvaluateConstant
int TestEvaluate_PureConstantExpression() {
    MakeExprStore(32);

    EvalResult r = EvaluateConstant(C(0xABCD1234));

    BF_TEST(r.status == EvalStatus::Success);
    BF_TEST(EqualBits(r.value, 0xABCD1234ULL));
    return 0;
}

int TestEvaluate_NestedExpression() {
    MakeExprStore(32);

    auto add = C(10) + C(20);
    auto sub = C(7) - C(3);
    auto mul = add * sub;
    auto expr = mul ^ C(0x55);

    EvalResult r = EvaluateConstant(expr, 16);

    BF_TEST(r.status == EvalStatus::Success);
    BF_TEST(EqualBits(r.value, 0x2DULL));
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

    BF_TEST(r8.status == EvalStatus::Success);
    BF_TEST(EqualBits(r8.value, 0x34ULL));

    BF_TEST(r16.status == EvalStatus::Success);
    BF_TEST(EqualBits(r16.value, 0x1234ULL));

    BF_TEST(r32.status == EvalStatus::Success);
    BF_TEST(EqualBits(r32.value, 0xFFFF1234ULL));

    BF_TEST(r64.status == EvalStatus::Success);
    BF_TEST(EqualBits(r64.value, 0xFFFF1234ULL));
    return 0;
}

int TestEvaluate_ShiftRotateEdgeCases() {
    MakeExprStore(32);

    auto v = C(0x81);

    auto z = C(0);
    BF_TEST(Eval(v << z, 0x81ULL));
    BF_TEST(Eval(v >> z, 0x81ULL));
    BF_TEST(Eval(v.RotL(z), 0x81ULL));
    BF_TEST(Eval(v.RotR(z), 0x81ULL));

    auto w = C(8);
    BF_TEST(Eval(v << w, 0x8100ULL));
    BF_TEST(Eval(v >> w, 0x0ULL));
    BF_TEST(Eval(v.RotL(w), 0x8100ULL));
    BF_TEST(Eval(v.RotR(w), 0x81000000ULL));

    auto n = C(9);
    BF_TEST(Eval(v << n, 0x10200ULL));
    BF_TEST(Eval(v >> n, 0x0ULL));
    BF_TEST(Eval(v.RotL(n), 0x10200ULL));
    BF_TEST(Eval(v.RotR(n), 0x40800000ULL));
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

    BF_TEST(rDiv.status == EvalStatus::DivisionByZero);
    BF_TEST(rMod.status == EvalStatus::ModuloByZero);
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

    BF_TEST(rLeaf.status == EvalStatus::NotConstant);
    BF_TEST(rMixed.status == EvalStatus::NotConstant);
    BF_TEST(rNested.status == EvalStatus::NotConstant);
    return 0;
}

int TestEvaluate_InvalidBitWidth() {
    MakeExprStore(32);

    auto c = C(5);

    EvalResult r0 = EvaluateConstant(c, 0);

    BF_TEST(r0.status == EvalStatus::InvalidBitWidth);
    BF_TEST(EvaluateConstant(c, 65).status == EvalStatus::Success);
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

    BF_TEST(r.status == EvalStatus::Success);
    BF_TEST(r.value.BitWidth() == 128U);
    BF_TEST(EqualBits(r.value, ((0x102ULL + 0x102ULL) % 257ULL)));

    auto rotr = c2.RotR(1);
    auto rotrWide = EvaluateConstant(rotr, 128);
    BF_TEST(rotrWide.status == EvalStatus::Success);
    BF_TEST(rotrWide.value.Shr(127).ToChunk() == 1ULL);
    return 0;
}
#pragma endregion

#pragma region IsFullyConstant
int TestDetect_ConstLeaf() {
    MakeExprStore(32);
    BF_TEST(IsFullyConstant(C(42)));
    return 0;
}

int TestDetect_VarLeafFalse() {
    MakeExprStore(32);
    BF_TEST(!IsFullyConstant(V()));
    return 0;
}

int TestDetect_AllChildrenConstant() {
    MakeExprStore(32);
    auto expr = C(10) + C(20);

    BF_TEST(IsFullyConstant(expr));
    return 0;
}

int TestDetect_AnyChildNonConstantFalse() {
    MakeExprStore(32);
    auto expr = C(10) + V();

    BF_TEST(!IsFullyConstant(expr));
    return 0;
}
#pragma endregion

int main() {
    BF_RUN_TEST(TestEvaluate_PureConstantExpression);
    BF_RUN_TEST(TestEvaluate_NestedExpression);
    BF_RUN_TEST(TestEvaluate_BitWidthVariations_8_16_32_64);
    BF_RUN_TEST(TestEvaluate_ShiftRotateEdgeCases);
    BF_RUN_TEST(TestEvaluate_DivisionModuloByZero);
    BF_RUN_TEST(TestEvaluate_NotConstantCases);
    BF_RUN_TEST(TestEvaluate_InvalidBitWidth);
    BF_RUN_TEST(TestEvaluate_WideBitWidth_UsesBfUintPath);

    BF_RUN_TEST(TestDetect_ConstLeaf);
    BF_RUN_TEST(TestDetect_VarLeafFalse);
    BF_RUN_TEST(TestDetect_AllChildrenConstant);
    BF_RUN_TEST(TestDetect_AnyChildNonConstantFalse);
    return 0;
}