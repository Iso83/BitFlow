#include <BitFlow/core/rules/RulePipeline.h>
#include <BitFlow/io/ExprLatex.h>
#include <BitFlow/io/ExprParser.h>
#include <BitFlow/io/ExprPrinter.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>
#include <TestAssert.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::IO;

static int Run_Parse_Latex(const std::string input) {
    MakeExprStore(32);

    RuleEngine engine = BuildExplore();

    auto parse = Parse(input);

    BF_SAFE_REWRITE(r, Rewrite(engine, parse.root, &parse.names));
    BF_TEST(!ToLatex(r, parse.names).empty());

    return 0;
}

#define BF_Test_Parse_Latex(input) BF_TEST(Run_Parse_Latex(input) == 0)

int main() {
    return 0;
}