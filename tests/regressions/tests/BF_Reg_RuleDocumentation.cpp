#include "DocTestUtils.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Rules;

static const DocExample g_examples[] = {

#pragma region Normalize

    {.rule = Normalize::Get_Order_Rule().key, .input = "b + a", .expected = "a + b"},

#pragma endregion

#pragma region NormalizeBitwise

    {.rule = Normalize::Bitwise::Get_RotateModulo_Rule().key, .input = "u32(a) <<< 32", .expected = "a"},

#pragma endregion

#pragma region SimplifyArithmetic

    {.rule = Simplify::Arithmetic::Get_AddZero_Rule().key, .input = "a + 0", .expected = "a"},

    {.rule = Simplify::Arithmetic::Get_MulOne_Rule().key, .input = "a * 1", .expected = "a"},

    {.rule = Simplify::Arithmetic::Get_MulOne_Rule().key, .input = "1 * 1 * a", .expected = "a"},

    {.rule = Simplify::Arithmetic::Get_PowOne_Rule().key, .input = "a**1", .expected = "a"},

    {.rule = Simplify::Arithmetic::Get_MulZero_Rule().key, .input = "a * 0", .expected = "0"},

    {.rule = Simplify::Arithmetic::Get_PowZero_Rule().key, .input = "a**0", .expected = "1"},

    {.rule = Simplify::Arithmetic::Get_SubZero_Rule().key, .input = "a - 0", .expected = "a"},

    {.rule = Simplify::Arithmetic::Get_SubSelf_Rule().key, .input = "a - a", .expected = "0"},

    {.rule = Simplify::Arithmetic::Get_DivOne_Rule().key, .input = "a / 1", .expected = "a"},

    {.rule = Simplify::Arithmetic::Get_DivSelf_Rule().key, .input = "a / a", .expected = "1"},

    {.rule = Simplify::Arithmetic::Get_ModOne_Rule().key, .input = "a % 1", .expected = "0"},

    {.rule = Simplify::Arithmetic::Get_ModSelf_Rule().key, .input = "a % a", .expected = "0"},

    {.rule = Simplify::Arithmetic::Get_ShiftZero_Rule().key, .input = "a << 0", .expected = "a"},

    {.rule = Simplify::Arithmetic::Get_RotateZero_Rule().key, .input = "a <<< 0", .expected = "a"},

    {.rule = Simplify::Arithmetic::Get_NegNeg_Rule().key, .input = "-(-a)", .expected = "a"},

    {.rule = Simplify::Arithmetic::Get_SubNeg_Rule().key, .input = "a - (-b)", .expected = "a + b"},

#pragma endregion

#pragma region SimplifyBitwise

    {.rule = Simplify::Bitwise::Get_XorZero_Rule().key, .input = "a ^ 0", .expected = "a"},

    {.rule = Simplify::Bitwise::Get_AndCancel_Rule().key, .input = "a & a", .expected = "a"},

    {.rule = Simplify::Bitwise::Get_OrCancel_Rule().key, .input = "a | a", .expected = "a"},

    {.rule = Simplify::Bitwise::Get_XorCancel_Rule().key, .input = "a ^ a", .expected = "0"},

    {.rule = Simplify::Bitwise::Get_Not_Rule().key, .input = "~(~a)", .expected = "a"},

    {.rule = Simplify::Bitwise::Get_Complement_Rule().key, .input = "a & ~a", .expected = "0"},

    {.rule = Simplify::Bitwise::Get_AndZeroDominance_Rule().key, .input = "a & 0", .expected = "0"},

    {.rule = Simplify::Bitwise::Get_OrZeroIdentity_Rule().key, .input = "a | 0", .expected = "a"},

#pragma endregion

};

int main() {
    for (const auto& ex : g_examples) {
        std::cout << "\n=== " << ex.rule.value << " ===\n";
        BF_TEST(ValidateDocExample(ex) == 0);
    }

    return 0;
}