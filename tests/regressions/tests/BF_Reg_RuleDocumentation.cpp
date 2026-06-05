#include "DocTestUtils.h"

#include <filesystem>
#include <fstream>
#include <memory>
#include <regex>
#include <string>
#include <vector>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Rules;

std::vector<Rule> BuildRulesInDeclarationOrder() {
    return {
#pragma region Normalize
        Normalize::Get_Flatten_Rule(),
        Normalize::Get_Order_Rule(),
#pragma endregion

#pragma region Normalize Arithmetic
        Normalize::Arithmetic::Get_AddNegToSub_Rule(),
        Normalize::Arithmetic::Get_SubToNeg_Rule(),
#pragma endregion

#pragma region Normalize Bitwise
        Normalize::Bitwise::Get_RotateModulo_Rule(),
#pragma endregion

#pragma region Simplify Arithmetic
        Simplify::Arithmetic::Get_AddZero_Rule(),
        Simplify::Arithmetic::Get_MulOne_Rule(),
        Simplify::Arithmetic::Get_PowOne_Rule(),
        Simplify::Arithmetic::Get_MulZero_Rule(),
        Simplify::Arithmetic::Get_PowZero_Rule(),
        Simplify::Arithmetic::Get_SubZero_Rule(),
        Simplify::Arithmetic::Get_SubSelf_Rule(),
        Simplify::Arithmetic::Get_DivOne_Rule(),
        Simplify::Arithmetic::Get_DivSelf_Rule(),
        Simplify::Arithmetic::Get_ModOne_Rule(),
        Simplify::Arithmetic::Get_ModSelf_Rule(),
        Simplify::Arithmetic::Get_ShiftZero_Rule(),
        Simplify::Arithmetic::Get_RotateZero_Rule(),
        Simplify::Arithmetic::Get_ShiftRotateConstantFold_Rule(),
        Simplify::Arithmetic::Get_NegNeg_Rule(),
        Simplify::Arithmetic::Get_NegPowEven_Rule(),
        Simplify::Arithmetic::Get_SubNeg_Rule(),
        Simplify::Arithmetic::Get_AddFold_Rule(),
        Simplify::Arithmetic::Get_SubConstFold_Rule(),
        Simplify::Arithmetic::Get_SubAddSelfCancel_Rule(),
        Simplify::Arithmetic::Get_SubMulLinearCancel_Rule(),
        Simplify::Arithmetic::Get_MulDivConstantReduction_Rule(),
        Simplify::Arithmetic::Get_MulToPow_Rule(),
        Simplify::Arithmetic::Get_CombineMulPow_Rule(),
        Simplify::Arithmetic::Get_CombineConstants_Rule(),
#pragma endregion

#pragma region Simplify Bitwise
        Simplify::Bitwise::Get_XorZero_Rule(),
        Simplify::Bitwise::Get_AndFold_Rule(),
        Simplify::Bitwise::Get_OrFold_Rule(),
        Simplify::Bitwise::Get_XorFold_Rule(),
        Simplify::Bitwise::Get_XorCancel_Rule(),
        Simplify::Bitwise::Get_Not_Rule(),
        Simplify::Bitwise::Get_NotPushdown_Rule(),
        Simplify::Bitwise::Get_NotXor_Rule(),
        Simplify::Bitwise::Get_Idempotent_Rule(),
        Simplify::Bitwise::Get_Complement_Rule(),
        Simplify::Bitwise::Get_AndXorReduction_Rule(),
        Simplify::Bitwise::Get_XorAndReduction_Rule(),
        Simplify::Bitwise::Get_XorAndNotReduction_Rule(),
#pragma endregion

#pragma region Factorize Arithmetic
        Factorize::Arithmetic::Get_AddLinearMultiplicity_Rule(),
        Factorize::Arithmetic::Get_AddCommonFactor_Rule(),
        Factorize::Arithmetic::Get_PerfectSquare_Rule(),
        Factorize::Arithmetic::Get_DifferenceOfSquares_Rule(),
        Factorize::Arithmetic::Get_PromoteFactorsToPower_Rule(),
        Factorize::Arithmetic::Get_CommonFactorCancel_PowTerms_Rule(),
        Factorize::Arithmetic::Get_CommonFactorCancel_Rule(),
        Factorize::Arithmetic::Get_SubCommonDenominator_Rule(),
        Factorize::Arithmetic::Get_AddCommonDenominator_Rule(),
        Factorize::Arithmetic::Get_MulFractionNumerator_Rule(),
        Factorize::Arithmetic::Get_DivFractionDenominator_Rule(),
#pragma endregion

#pragma region Factorize Bitwise
        Factorize::Bitwise::Get_XorAnd_Rule(),
        Factorize::Bitwise::Get_AndAbsorb_Rule(),
        Factorize::Bitwise::Get_OrAbsorb_Rule(),
        Factorize::Bitwise::Get_DeMorganAnd_Rule(),
        Factorize::Bitwise::Get_DeMorganOr_Rule(),
        Factorize::Bitwise::Get_Distribute_Rule(),
        Factorize::Bitwise::Get_DistributeAndOverOr_Rule(),
        Factorize::Bitwise::Get_DistributeOrOverAnd_Rule(),
#pragma endregion
    };
}

std::vector<DocExample> BuildDocExamples() {
    return {
#pragma region Normalize
        {.rule = Normalize::Get_Flatten_Rule().key, .input = "(a + b) + c", .expected = "a + b + c"},

        {.rule = Normalize::Get_Order_Rule().key, .input = "b + a", .expected = "a + b"},
#pragma endregion

#pragma region Normalize Arithmetic
        {.rule = Normalize::Arithmetic::Get_AddNegToSub_Rule().key, .input = "x + (-y)", .expected = "x - y"},
        {.rule = Normalize::Arithmetic::Get_AddNegToSub_Rule().key, .input = "1 + (-a)", .expected = "1 - a"},

        {.rule = Normalize::Arithmetic::Get_SubToNeg_Rule().key,
         .input = "1 - a",
         .expected = "-(a - 1)",
         .expand = true},

        {.rule = Normalize::Arithmetic::Get_SubToNeg_Rule().key,
         .input = "2 - b",
         .expected = "-(b - 2)",
         .expand = true},
#pragma endregion

#pragma region Normalize Bitwise
        {.rule = Normalize::Bitwise::Get_RotateModulo_Rule().key, .input = "u32(x) <<< 32", .expected = "x"},
        {.rule = Normalize::Bitwise::Get_RotateModulo_Rule().key, .input = "u32(x) >>> 40", .expected = "x >>> 8"},
#pragma endregion

#pragma region Simplify Arithmetic
        {.rule = Simplify::Arithmetic::Get_AddZero_Rule().key, .input = "x + 0", .expected = "x"},

        {.rule = Simplify::Arithmetic::Get_MulOne_Rule().key, .input = "x * 1", .expected = "x"},
        {.rule = Simplify::Arithmetic::Get_MulOne_Rule().key, .input = "1 * 1 * x", .expected = "x"},

        {.rule = Simplify::Arithmetic::Get_PowOne_Rule().key, .input = "x**1", .expected = "x"},
        {.rule = Simplify::Arithmetic::Get_PowOne_Rule().key, .input = "(a + b)**1", .expected = "a + b"},

        {.rule = Simplify::Arithmetic::Get_MulZero_Rule().key, .input = "0 * x", .expected = "0"},

        {.rule = Simplify::Arithmetic::Get_PowZero_Rule().key, .input = "x**0", .expected = "1"},
        {.rule = Simplify::Arithmetic::Get_PowZero_Rule().key, .input = "(a + b)**0", .expected = "1"},

        {.rule = Simplify::Arithmetic::Get_SubZero_Rule().key, .input = "x - 0", .expected = "x"},

        {.rule = Simplify::Arithmetic::Get_SubSelf_Rule().key, .input = "x - x", .expected = "0"},

        {.rule = Simplify::Arithmetic::Get_DivOne_Rule().key, .input = "x / 1", .expected = "x"},

        {.rule = Simplify::Arithmetic::Get_DivSelf_Rule().key, .input = "x / x", .expected = "1"},

        {.rule = Simplify::Arithmetic::Get_ModOne_Rule().key, .input = "x % 1", .expected = "0"},

        {.rule = Simplify::Arithmetic::Get_ModSelf_Rule().key, .input = "x % x", .expected = "0"},

        {.rule = Simplify::Arithmetic::Get_ShiftZero_Rule().key, .input = "x << 0", .expected = "x"},

        {.rule = Simplify::Arithmetic::Get_RotateZero_Rule().key, .input = "x <<< 0", .expected = "x"},

        {.rule = Simplify::Arithmetic::Get_ShiftRotateConstantFold_Rule().key,
         .input = "u8(129) <<< 1",
         .expected = "3"},
        {.rule = Simplify::Arithmetic::Get_ShiftRotateConstantFold_Rule().key,
         .input = "u8(128) >> 7",
         .expected = "1"},

        {.rule = Simplify::Arithmetic::Get_NegNeg_Rule().key, .input = "-(-x)", .expected = "x"},

        {.rule = Simplify::Arithmetic::Get_NegPowEven_Rule().key, .input = "(-x)**2", .expected = "x ** 2"},
        {.rule = Simplify::Arithmetic::Get_NegPowEven_Rule().key, .input = "(-(a - 1))**2", .expected = "(a - 1) ** 2"},
        {.rule = Simplify::Arithmetic::Get_NegPowEven_Rule().key, .input = "(-x)**8", .expected = "x ** 8"},

        {.rule = Simplify::Arithmetic::Get_SubNeg_Rule().key, .input = "x - (-y)", .expected = "x + y"},
        {.rule = Simplify::Arithmetic::Get_SubNeg_Rule().key, .input = "x - (-(y + z))", .expected = "x + y + z"},

        {.rule = Simplify::Arithmetic::Get_AddFold_Rule().key, .input = "x + 10 + 20", .expected = "30 + x"},

        {.rule = Simplify::Arithmetic::Get_SubConstFold_Rule().key, .input = "(x + 8) - 1", .expected = "7 + x"},

        {.rule = Simplify::Arithmetic::Get_SubAddSelfCancel_Rule().key,
         .input = "(a + b + c) - b",
         .expected = "a + c"},
        {.rule = Simplify::Arithmetic::Get_SubAddSelfCancel_Rule().key,
         .input = "(a + b) - (b - 2)",
         .expected = "2 + a"},
        {.rule = Simplify::Arithmetic::Get_SubAddSelfCancel_Rule().key, .input = "(1 + b) - (b - 2)", .expected = "3"},

        {.rule = Simplify::Arithmetic::Get_SubMulLinearCancel_Rule().key, .input = "x * 5 - x", .expected = "4 * x"},

        {.rule = Simplify::Arithmetic::Get_MulDivConstantReduction_Rule().key,
         .input = "x * 12 / 3",
         .expected = "4 * x"},

        {.rule = Simplify::Arithmetic::Get_MulToPow_Rule().key, .input = "x * x", .expected = "x ** 2"},

        {.rule = Simplify::Arithmetic::Get_CombineMulPow_Rule().key, .input = "x * x**2", .expected = "x ** 3"},
        {.rule = Simplify::Arithmetic::Get_CombineMulPow_Rule().key, .input = "x**a * x", .expected = "x ** (1 + a)"},
        {.rule = Simplify::Arithmetic::Get_CombineMulPow_Rule().key,
         .input = "x**a * x**b",
         .expected = "x ** (a + b)"},

        //{.rule = Simplify::Arithmetic::Get_CombineConstants_Rule().key, .input= "2 + 3 + 4", .expected= "9"}, // use
        //--> CORE.SIMPLIFY.ARITHMETIC.ADD_FOLD
        {.rule = Simplify::Arithmetic::Get_CombineConstants_Rule().key, .input = "20 / 4", .expected = "5"},
        {.rule = Simplify::Arithmetic::Get_CombineConstants_Rule().key, .input = "20 * 5", .expected = "100"},
        {.rule = Simplify::Arithmetic::Get_CombineConstants_Rule().key, .input = "20 % 6", .expected = "2"},
#pragma endregion

#pragma region Simplify Bitwise
        {.rule = Simplify::Bitwise::Get_XorZero_Rule().key, .input = "0 ^ x", .expected = "x"},

        {.rule = Simplify::Bitwise::Get_AndFold_Rule().key, .input = "u8(x) & u8(255) & u8(15)", .expected = "15 & x"},
        {.rule = Simplify::Bitwise::Get_AndFold_Rule().key, .input = "x & 0", .expected = "0"},
        {.rule = Simplify::Bitwise::Get_AndFold_Rule().key, .input = "255 & 15", .expected = "15"},

        {.rule = Simplify::Bitwise::Get_OrFold_Rule().key, .input = "x | 1 | 2", .expected = "3 | x"},
        {.rule = Simplify::Bitwise::Get_OrFold_Rule().key, .input = "1 | 2 | a | a", .expected = "3 | a"},
        {.rule = Simplify::Bitwise::Get_OrFold_Rule().key, .input = "x | 0", .expected = "x"},
        {.rule = Simplify::Bitwise::Get_OrFold_Rule().key, .input = "u8(x) | u8(255)", .expected = "255"},

        {.rule = Simplify::Bitwise::Get_XorFold_Rule().key, .input = "x ^ 1 ^ 2", .expected = "3 ^ x"},

        {.rule = Simplify::Bitwise::Get_XorCancel_Rule().key, .input = "x ^ x", .expected = "0"},

        {.rule = Simplify::Bitwise::Get_Not_Rule().key, .input = "~(~x)", .expected = "x"},

        {.rule = Simplify::Bitwise::Get_NotPushdown_Rule().key, .input = "~(~x & y)", .expected = "x | ~y"},

        {.rule = Simplify::Bitwise::Get_NotXor_Rule().key, .input = "~(u8(x) ^ u8(y))", .expected = "255 ^ x ^ y"},

        {.rule = Simplify::Bitwise::Get_Idempotent_Rule().key, .input = "x | x", .expected = "x"},
        {.rule = Simplify::Bitwise::Get_Idempotent_Rule().key, .input = "x & x", .expected = "x"},

        {.rule = Simplify::Bitwise::Get_Complement_Rule().key, .input = "x & ~x", .expected = "0"},
        {.rule = Simplify::Bitwise::Get_Complement_Rule().key, .input = "u8(x) | ~u8(x)", .expected = "255"},

        {.rule = Simplify::Bitwise::Get_AndXorReduction_Rule().key, .input = "(x ^ y) & x", .expected = "x & ~y"},

        {.rule = Simplify::Bitwise::Get_XorAndReduction_Rule().key, .input = "x ^ (x & y)", .expected = "x & ~y"},
        {.rule = Simplify::Bitwise::Get_XorAndReduction_Rule().key, .input = "y ^ (y & x)", .expected = "y & ~x"},

        {.rule = Simplify::Bitwise::Get_XorAndNotReduction_Rule().key, .input = "(a ^ b) & ~a", .expected = "b & ~a"},
        {.rule = Simplify::Bitwise::Get_XorAndNotReduction_Rule().key,
         .input = "~a & (a ^ b ^ c)",
         .expected = "~a & (b ^ c)"},
#pragma endregion

#pragma region Factorize Arithmetic
        {.rule = Factorize::Arithmetic::Get_AddLinearMultiplicity_Rule().key,
         .input = "x + x + x",
         .expected = "3 * x"},
        {.rule = Factorize::Arithmetic::Get_AddLinearMultiplicity_Rule().key,
         .input = "2 * x + x",
         .expected = "3 * x"},
        {.rule = Factorize::Arithmetic::Get_AddLinearMultiplicity_Rule().key,
         .input = "x + (2 * x - 4)",
         .expected = "3 * x - 4"},

        {.rule = Factorize::Arithmetic::Get_AddCommonFactor_Rule().key,
         .input = "a * x + a * y",
         .expected = "a * (x + y)"},

        {.rule = Factorize::Arithmetic::Get_PerfectSquare_Rule().key,
         .input = "a**2 + 2*a*b + b**2",
         .expected = "(a + b) ** 2"},
        {.rule = Factorize::Arithmetic::Get_PerfectSquare_Rule().key,
         .input = "a**2 - 2*a*b + b**2",
         .expected = "(a - b) ** 2"},
        {.rule = Factorize::Arithmetic::Get_PerfectSquare_Rule().key,
         .input = "a**2 - 6*a + 9",
         .expected = "(3 - a) ** 2"},

        {.rule = Factorize::Arithmetic::Get_DifferenceOfSquares_Rule().key,
         .input = "a**2 - b**2",
         .expected = "(a + b) * (a - b)"},
        {.rule = Factorize::Arithmetic::Get_DifferenceOfSquares_Rule().key,
         .input = "(a + 1)**2 - (a - 2)**2",
         .expected = "(1 + a + (a - 2)) * (1 + a - (a - 2))",
         .disabledRules = {Simplify::Arithmetic::SubAddSelfCancel, Factorize::Arithmetic::AddLinearMultiplicity,
                           Factorize::Arithmetic::AddCommonFactor}},

        {.rule = Factorize::Arithmetic::Get_PromoteFactorsToPower_Rule().key,
         .input = "a * b * c * (a * b)**2",
         .expected = "c * (a * b) ** 3"},
        {.rule = Factorize::Arithmetic::Get_PromoteFactorsToPower_Rule().key,
         .input = "a * b * c * d * (a * b * c)**5",
         .expected = "d * (a * b * c) ** 6"},

        {.rule = Factorize::Arithmetic::Get_CommonFactorCancel_PowTerms_Rule().key,
         .input = "a**5 * 2 / (3 * a**5)",
         .expected = "2 / 3"},
        {.rule = Factorize::Arithmetic::Get_CommonFactorCancel_PowTerms_Rule().key,
         .input = "a**8 * 2 / (3 * a**5)",
         .expected = "2 * a ** 3 / 3"},

        {.rule = Factorize::Arithmetic::Get_CommonFactorCancel_Rule().key,
         .input = "a * b * 2 / (b * 3)",
         .expected = "2 * a / 3"},

        {.rule = Factorize::Arithmetic::Get_SubCommonDenominator_Rule().key,
         .input = "a / c - b / c",
         .expected = "(a - b) / c"},
        {.rule = Factorize::Arithmetic::Get_SubCommonDenominator_Rule().key,
         .input = "40 + 5 / 8 - 3 / 8",
         .expected = "40 + (5 - 3) / 8",
         .disabledRules = {Simplify::Arithmetic::CombineConstants}},

        {.rule = Factorize::Arithmetic::Get_AddCommonDenominator_Rule().key,
         .input = "a / x + b / x",
         .expected = "(a + b) / x"},

        {.rule = Factorize::Arithmetic::Get_MulFractionNumerator_Rule().key,
         .input = "2 * (3 / 8)",
         .expected = "2 * 3 / 8",
         .disabledRules = {Simplify::Arithmetic::CombineConstants}},
        {.rule = Factorize::Arithmetic::Get_MulFractionNumerator_Rule().key,
         .input = "a / b * c",
         .expected = "a * c / b"},

        {.rule = Factorize::Arithmetic::Get_DivFractionDenominator_Rule().key,
         .input = "a / (b / c)",
         .expected = "a * c / b"},
#pragma endregion

#pragma region Factorize Bitwise
        {.rule = Factorize::Bitwise::Get_XorAnd_Rule().key, .input = "(y & x) ^ (z & x)", .expected = "x & (y ^ z)"},
        {.rule = Factorize::Bitwise::Get_XorAnd_Rule().key,
         .input = "x ^ (x & y) ^ (x & z)",
         .expected = "x & (z ^ ~y)"},

        {.rule = Factorize::Bitwise::Get_AndAbsorb_Rule().key, .input = "x & (x | y)", .expected = "x"},

        {.rule = Factorize::Bitwise::Get_OrAbsorb_Rule().key, .input = "x | (x & y)", .expected = "x"},

        {.rule = Factorize::Bitwise::Get_DeMorganAnd_Rule().key, .input = "~a | ~b", .expected = "~(a & b)"},

        {.rule = Factorize::Bitwise::Get_DeMorganOr_Rule().key, .input = "~a & ~b", .expected = "~(a | b)"},

        {.rule = Factorize::Bitwise::Get_Distribute_Rule().key,
         .input = "x & (y ^ z)",
         .expected = "y & x ^ z & x",
         .expand = true},

        {.rule = Factorize::Bitwise::Get_DistributeAndOverOr_Rule().key,
         .input = "(a & b) | (a & c)",
         .expected = "a & (b | c)"},

        {.rule = Factorize::Bitwise::Get_DistributeOrOverAnd_Rule().key,
         .input = "(a | b) & (a | c)",
         .expected = "a | b & c"},
#pragma endregion
    };
}

std::filesystem::path FindCoreArchitectureDoc() {
    std::filesystem::path path = __FILE__;
    while (!path.empty()) {
        const auto candidate = path / "docs/core/core_architecture.md";
        if (std::filesystem::exists(candidate))
            return candidate;

        path = path.parent_path();
    }

    return "docs/core/core_architecture.md";
}

std::vector<std::string> ReadDocumentedRuleKeys() {
    const auto path = FindCoreArchitectureDoc();
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cout << "\nUnable to open rule documentation: " << path << "\n";
        return {};
    }

    std::vector<std::string> keys;
    std::string line;
    const std::regex heading(R"(^### (CORE\.[A-Z0-9_\.]+)\s*$)");

    while (std::getline(in, line)) {
        std::smatch match;
        if (std::regex_match(line, match, heading))
            keys.push_back(match[1].str());
    }

    return keys;
}

int ValidateRuleOrder(const std::vector<Rule>& rules) {
    const auto documented = ReadDocumentedRuleKeys();

    BF_TEST(documented.size() == rules.size());
    const size_t count = std::min(documented.size(), rules.size());

    for (size_t i = 0; i < count; ++i) {
        if (documented[i] != rules[i].key.value) {
            std::cout << "\nRule documentation order mismatch at index " << i << "\n"
                      << "  Documented: " << documented[i] << "\n"
                      << "  Expected  : " << rules[i].key.value << "\n";
            BF_TEST(false);
        }
    }

    return 0;
}

int main() {
    const auto rules = BuildRulesInDeclarationOrder();
    BF_TEST(ValidateRuleOrder(rules) == 0);

    for (const auto& ex : BuildDocExamples()) {
        std::cout << "\n=== " << ex.rule.value << " ===\n";
        auto engine = ex.expand ? BuildExpand() : BuildExplore();
        for (auto key : ex.disabledRules)
            engine.RemoveRule(key);

        BF_TEST(engine.Contains(ex.rule));
        BF_RUN_TEST(ValidateDocExample, engine, ex, ex.trace);
    }

    return 0;
}