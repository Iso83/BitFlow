#include <BitFlow/core/rules/RulePipeline.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Rules;

namespace {

int Test_Validate_All_Safe_Profiles() {
    const auto normalize = ValidateProfile("normalize");
    BF_TEST(normalize.valid);

    const auto simplifyBitwise = ValidateProfile("simplify_bitwise");
    BF_TEST(simplifyBitwise.valid);

    const auto simplifyArithmetic = ValidateProfile("simplify_arithmetic");
    BF_TEST(simplifyArithmetic.valid);

    const auto simplifyFull = ValidateProfile("simplify_full_safe");
    BF_TEST(simplifyFull.valid);

    const auto factorizeBitwise = ValidateProfile("factorize_bitwise_safe");
    BF_TEST(factorizeBitwise.valid);

    const auto factorizeArithmetic = ValidateProfile("factorize_arithmetic_safe");
    BF_TEST(factorizeArithmetic.valid);

    const auto factorizeFull = ValidateProfile("factorize_full_safe");
    BF_TEST(factorizeFull.valid);

    const auto expandBitwise = ValidateProfile("expand_bitwise");
    BF_TEST(expandBitwise.valid);

    return 0;
}

int Test_Validate_Explore_Profile() {
    const auto explore = ValidateProfile("explore");
    BF_TEST(explore.valid);
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(Test_Validate_All_Safe_Profiles);
    BF_RUN_TEST(Test_Validate_Explore_Profile);
    return 0;
}
