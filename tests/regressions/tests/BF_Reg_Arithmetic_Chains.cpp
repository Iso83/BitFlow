#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestModZero_GuardLeftZeroStaysMod() {
    MakeExprStore(32);
    RuleEngine engine = BuildExplore();

    auto x = V("x");
    auto expr = C(0) % x;

    BF_SAFE_REWRITE(r, Rewrite(engine, expr));

    BF_TEST(r == expr);
    return 0;
}

int Test_ArithmeticChain_Canonicalization() {
    MakeExprStore(32);
    RuleEngine engine = BuildExplore();

    const auto a = V("a");

    BF_SAFE_REWRITE(r, Rewrite(engine, (((a + C(5)) + C(6)) - C(8)) - (-C(7))));

    BF_TEST(ToString(r) == "10 + a");
    return 0;
}

int Test_ArithmeticChain_FractionMerge() {
    MakeExprStore(32);
    RuleEngine engine = BuildExplore();

    BF_SAFE_REWRITE(r, Rewrite(engine, C(5) / 8 + C(3) / 8));

    BF_TEST(ToString(r) == "1");
    return 0;
}

int Test_ArithmeticChain_FractionSubMul() {
    MakeExprStore(32);
    RuleEngine engine = BuildExplore();

    BF_SAFE_REWRITE(r, Rewrite(engine, C(5) / 8 - C(2) * C(3) / 8));

    BF_TEST(ToString(r) == "-1 / 8");
    return 0;
}

int main() {
    BF_RUN_TEST(TestModZero_GuardLeftZeroStaysMod);

    BF_RUN_TEST(Test_ArithmeticChain_Canonicalization);
    BF_RUN_TEST(Test_ArithmeticChain_FractionMerge);
    BF_RUN_TEST(Test_ArithmeticChain_FractionSubMul);
    return 0;
}
