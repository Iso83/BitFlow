#include <BitFlow/core/rules/RulePipeline.h>
#include <BitFlow/io/ExprLatex.h>
#include <BitFlow/io/ExprParser.h>
#include <BitFlow/io/ExprPrinter.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>
#include <TestAssert.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::IO;

int Test_OR_FOLD_NoRewriteLoop() {
    MakeExprStore(32);

    RuleEngine engine = BuildExplore();
    auto parse = Parse("7|(f+f)");

    BF_SAFE_REWRITE(r, Rewrite(engine, parse.root));
    BF_TEST(BitFlow::IO::ToString(r, parse.names) == "7 | (2 * f)");
    return 0;
}

int main() {
    BF_RUN_TEST(Test_OR_FOLD_NoRewriteLoop);
    return 0;
}