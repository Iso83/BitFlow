#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <SHA_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Testing::SHA;

namespace {

uint32_t RotR32(uint32_t x, uint32_t amount) {
    const uint32_t s = amount & 31U;
    if (s == 0U)
        return x;
    return (x >> s) | (x << (32U - s));
}

int TestCH_HelperMatchesBooleanDefinition() {
    Builder b;
    auto x = b.Const(0x12345678U);
    auto y = b.Const(0xF0F0F0F0U);
    auto z = b.Const(0x00FF00FFU);

    auto viaHelper = b.Ch(x, y, z);
    auto booleanForm = b.Xor({b.And(x, y), b.And(b.Not(x), z)});

    const auto evalA = BitFlow::Core::Eval::EvaluateConstant(viaHelper, 32);
    const auto evalB = BitFlow::Core::Eval::EvaluateConstant(booleanForm, 32);

    BF_TEST(evalA.status == BitFlow::Core::Eval::EvalStatus::Success);
    BF_TEST(evalB.status == BitFlow::Core::Eval::EvalStatus::Success);
    BF_TEST(evalA.value == evalB.value);
    return 0;
}

int TestMAJ_HelperMatchesBooleanDefinition() {
    Builder b;
    auto x = b.Const(0x01234567U);
    auto y = b.Const(0x89ABCDEFU);
    auto z = b.Const(0x0F0FF0F0U);

    auto viaHelper = b.Maj(x, y, z);
    auto booleanForm = b.Xor({b.And(x, y), b.And(x, z), b.And(y, z)});

    const auto evalA = BitFlow::Core::Eval::EvaluateConstant(viaHelper, 32);
    const auto evalB = BitFlow::Core::Eval::EvaluateConstant(booleanForm, 32);

    BF_TEST(evalA.status == BitFlow::Core::Eval::EvalStatus::Success);
    BF_TEST(evalB.status == BitFlow::Core::Eval::EvalStatus::Success);
    BF_TEST(evalA.value == evalB.value);
    return 0;
}

int TestBigSigma1_UsesExpectedRotateRecipe() {
    Builder b;
    auto x = b.Var();
    auto sigma = b.BigSigma1(x);

    BF_TEST(sigma->op == OpType::Xor);
    BF_TEST(sigma->inputs.size() == 3);

    const uint32_t expected[3] = {6U, 11U, 25U};
    for (size_t i = 0; i < 3; ++i) {
        BF_TEST(sigma->inputs[i]->op == OpType::RotR);
        BF_TEST(sigma->inputs[i]->inputs.size() == 2);
        BF_TEST(sigma->inputs[i]->inputs[0] == x);
        BF_TEST(sigma->inputs[i]->inputs[1]->op == OpType::Const);
        BF_TEST(sigma->inputs[i]->inputs[1]->constValue == expected[i]);
    }

    return 0;
}

int TestRoundFragments_AreDirectlyEvaluatable() {
    Builder b;

    constexpr uint32_t a = 0x6A09E667U;
    constexpr uint32_t bVal = 0xBB67AE85U;
    constexpr uint32_t c = 0x3C6EF372U;
    constexpr uint32_t e = 0x510E527FU;
    constexpr uint32_t f = 0x9B05688CU;
    constexpr uint32_t g = 0x1F83D9ABU;
    constexpr uint32_t h = 0x5BE0CD19U;
    constexpr uint32_t k = 0x428A2F98U;
    constexpr uint32_t w = 0x00000000U;

    auto t1Expr = b.RoundT1(
        b.Const(h),
        b.Const(e),
        b.Const(f),
        b.Const(g),
        b.Const(k),
        b.Const(w));

    auto t2Expr = b.RoundT2(
        b.Const(a),
        b.Const(bVal),
        b.Const(c));

    const auto t1 = BitFlow::Core::Eval::EvaluateConstant(t1Expr, 32);
    const auto t2 = BitFlow::Core::Eval::EvaluateConstant(t2Expr, 32);

    BF_TEST(t1.status == BitFlow::Core::Eval::EvalStatus::Success);
    BF_TEST(t2.status == BitFlow::Core::Eval::EvalStatus::Success);

    const uint32_t sigma1 = RotR32(e, 6) ^ RotR32(e, 11) ^ RotR32(e, 25);
    const uint32_t ch = (e & f) ^ ((~e) & g);
    const uint32_t expectedT1 = h + sigma1 + ch + k + w;

    const uint32_t sigma0 = RotR32(a, 2) ^ RotR32(a, 13) ^ RotR32(a, 22);
    const uint32_t maj = (a & bVal) ^ (a & c) ^ (bVal & c);
    const uint32_t expectedT2 = sigma0 + maj;

    BF_TEST(static_cast<uint32_t>(t1.value) == expectedT1);
    BF_TEST(static_cast<uint32_t>(t2.value) == expectedT2);
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestCH_HelperMatchesBooleanDefinition);
    BF_RUN_TEST(TestMAJ_HelperMatchesBooleanDefinition);
    BF_RUN_TEST(TestBigSigma1_UsesExpectedRotateRecipe);
    BF_RUN_TEST(TestRoundFragments_AreDirectlyEvaluatable);
    return 0;
}
