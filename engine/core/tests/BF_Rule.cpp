#include <BitFlow/core/rules/Rule.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Rules;

int TestRule_Construct() {
    auto rule = Simplify::Arithmetic::Get_AddZero_Rule();

    BF_TEST(rule.key == Simplify::Arithmetic::AddZero);
    BF_TEST(rule.match != nullptr);
    BF_TEST(rule.rewrite != nullptr);

    return 0;
}

int TestRule_WithDependencies() {
    auto rule = Simplify::Bitwise::Get_XorAndReduction_Rule();

    BF_TEST(rule.key == Simplify::Bitwise::XorAndReduction);
    BF_TEST(rule.match != nullptr);
    BF_TEST(rule.rewrite != nullptr);

    BF_TEST(!rule.deps.empty());

    return 0;
}

int main() {
    BF_RUN_TEST(TestRule_Construct);
    BF_RUN_TEST(TestRule_WithDependencies);
    return 0;
}