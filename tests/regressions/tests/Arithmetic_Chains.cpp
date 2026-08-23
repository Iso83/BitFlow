#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

#include <BitFlow/engine/core/rules/RulePipeline.h>
#include <BitFlow/engine/io/ExprParser.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;
using namespace BitFlow::Engine::IO;

int TestModZero_GuardLeftZeroStaysMod() {
    MakeExprStore(32);
    RuleEngine engine = BuildExplore();

    auto x = V("x");
    auto expr = C(0) % x;

    BF_SAFE_REWRITE(r, BF_REWRITE(expr));

    CPPTEST_ASSERT(r == expr);
    return 0;
}

int Test_ArithmeticChain_Canonicalization() {
    MakeExprStore(32);
    RuleEngine engine = BuildExplore();

    const auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE((((a + C(5)) + C(6)) - C(8)) - (-C(7))));

    CPPTEST_ASSERT(ToString(r) == "10 + a");
    return 0;
}

int Test_ArithmeticChain_FractionMerge() {
    MakeExprStore(32);
    RuleEngine engine = BuildExplore();

    BF_SAFE_REWRITE(r, BF_REWRITE(C(5) / 8 + C(3) / 8));

    CPPTEST_ASSERT(ToString(r) == "1");
    return 0;
}

int Test_ArithmeticChain_FractionSubMul() {
    MakeExprStore(32);
    RuleEngine engine = BuildExplore();

    BF_SAFE_REWRITE(r, BF_REWRITE(C(5) / 8 - C(2) * C(3) / 8));

    CPPTEST_ASSERT(ToString(r) == "-1 / 8");
    return 0;
}

int Test_FractionChains() {
    MakeExprStore(32);
    RuleEngine engine = BuildExplore();

    auto parse = Parse("2 * (3/8) + 5/8 - 3/8");
    BF_SAFE_REWRITE(r, BF_REWRITE(parse.root));

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(r, parse.names) == "1");
    return 0;
}

int Test_NestedPowerReductions() {
    MakeExprStore(32);
    RuleEngine engine = BuildExplore();

    auto parse = Parse("a**5 * 2 - 3 * a**5");
    BF_SAFE_REWRITE(r, BF_REWRITE(parse.root));

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(r, parse.names) == "-a ** 5");
    return 0;
}

int Test_AffineChains() {
    MakeExprStore(32);
    RuleEngine engine = BuildExplore();

    auto parse = Parse("a + 5 + 6 - 8 - (-7)");
    BF_SAFE_REWRITE(r, BF_REWRITE(parse.root));

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(r, parse.names) == "10 + a");
    return 0;
}

int Test_MixedFractionCancellation() {
    MakeExprStore(32);
    RuleEngine engine = BuildExplore();

    auto parse = Parse("a**8 * 2 / (3 * a**5)");
    BF_SAFE_REWRITE(r, BF_REWRITE(parse.root));

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(r, parse.names) == "2 * a ** 3 / 3");
    return 0;
}

int Test_Rewrite_RevisitsGeneratedSubtrees() {
    MakeExprStore(32);

    RuleEngine engine = BuildExplore();

    BF_SAFE_REWRITE(r, BF_REWRITE(C(3) / C(8) - (C(7) / C(8) - C(4) / C(8))));

    CPPTEST_ASSERT(EqualChunkValue(r, 0u));

    return 0;
}

int main() {
    CPPTEST_RUN(TestModZero_GuardLeftZeroStaysMod);

    CPPTEST_RUN(Test_ArithmeticChain_Canonicalization);
    CPPTEST_RUN(Test_ArithmeticChain_FractionMerge);
    CPPTEST_RUN(Test_ArithmeticChain_FractionSubMul);

    CPPTEST_RUN(Test_FractionChains);
    CPPTEST_RUN(Test_NestedPowerReductions);
    CPPTEST_RUN(Test_AffineChains);
    CPPTEST_RUN(Test_MixedFractionCancellation);

    CPPTEST_RUN(Test_Rewrite_RevisitsGeneratedSubtrees);
    return 0;
}
