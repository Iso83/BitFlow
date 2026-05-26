#include <BitFlow/core/rules/RulePipeline.h>
#include <BitFlow/io/ExprParser.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::IO;

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

int Test_FractionChains() {
    MakeExprStore(32);
    RuleEngine engine = BuildExplore();

    auto parse = Parse("2 * (3/8) + 5/8 - 3/8");
    BF_SAFE_REWRITE(r, Rewrite(engine, parse.root));

    BF_TEST(BitFlow::IO::ToString(r, parse.names) == "1");
    return 0;
}

int Test_NestedPowerReductions() {
    MakeExprStore(32);
    RuleEngine engine = BuildExplore();

    auto parse = Parse("a**5 * 2 - 3 * a**5");
    BF_SAFE_REWRITE(r, Rewrite(engine, parse.root));

    BF_TEST(BitFlow::IO::ToString(r, parse.names) == "-a ** 5");
    return 0;
}

int Test_AffineChains() {
    MakeExprStore(32);
    RuleEngine engine = BuildExplore();

    auto parse = Parse("a + 5 + 6 - 8 - (-7)");
    BF_SAFE_REWRITE(r, Rewrite(engine, parse.root));

    BF_TEST(BitFlow::IO::ToString(r, parse.names) == "10 + a");
    return 0;
}

int Test_MixedFractionCancellation() {
    MakeExprStore(32);
    RuleEngine engine = BuildExplore();

    auto parse = Parse("a**8 * 2 / (3 * a**5)");
    BF_SAFE_REWRITE(r, Rewrite(engine, parse.root));

    BF_TEST(BitFlow::IO::ToString(r, parse.names) == "2 * a ** 3 / 3");
    return 0;
}

int Test_Rewrite_RevisitsGeneratedSubtrees() {
    MakeExprStore(32);

    RuleEngine engine = BuildExplore();

    BF_SAFE_REWRITE(r, Rewrite(engine, C(3) / C(8) - (C(7) / C(8) - C(4) / C(8))));

    BF_TEST(EqualChunkValue(r, 0u));

    return 0;
}

int main() {
    BF_RUN_TEST(TestModZero_GuardLeftZeroStaysMod);

    BF_RUN_TEST(Test_ArithmeticChain_Canonicalization);
    BF_RUN_TEST(Test_ArithmeticChain_FractionMerge);
    BF_RUN_TEST(Test_ArithmeticChain_FractionSubMul);

    BF_RUN_TEST(Test_FractionChains);
    BF_RUN_TEST(Test_NestedPowerReductions);
    BF_RUN_TEST(Test_AffineChains);
    BF_RUN_TEST(Test_MixedFractionCancellation);

    BF_RUN_TEST(Test_Rewrite_RevisitsGeneratedSubtrees);
    return 0;
}
