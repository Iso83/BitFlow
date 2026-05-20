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

static int Run_Parse_Latex(const std::string& input) {
    MakeExprStore(32);

    RuleEngine engine = BuildExplore();

    auto parse = Parse(input);

    BF_SAFE_REWRITE(r, Rewrite(engine, parse.root, &parse.names));

    const auto text = BitFlow::IO::ToString(r, parse.names);
    BF_TEST(!text.empty());

    auto parse2 = Parse(text);

    BF_SAFE_REWRITE(r2, Rewrite(engine, parse2.root, &parse2.names));

    BF_TEST(!ToLatex(r2, parse2.names).empty());

    return 0;
}

#define BF_Test_Parse_Latex(input) BF_TEST(Run_Parse_Latex(input) == 0)

int Test_Fuzzing() {
    BF_Test_Parse_Latex("a*a*a*a*a*a*a*a*a*a");
    BF_Test_Parse_Latex("a+a+a+a+a+a+a+a+a+a");
    BF_Test_Parse_Latex("a*a + a*a + a*a + a*a");
    BF_Test_Parse_Latex("a*a*a + a*a*a + a*a*a");
    BF_Test_Parse_Latex("(a+b)*(a+b)*(a+b)");
    BF_Test_Parse_Latex("a^b^(a+1)^c^(a+2)^a");
    BF_Test_Parse_Latex("(a*a)*(a*a)*(a*a)");
    BF_Test_Parse_Latex("a*a*a*a / (a*a)");
    BF_Test_Parse_Latex("(a*b)+(a*c)+(a*d)+(a*e)");
    BF_Test_Parse_Latex("(a+b+c+d)*(a+b+c+d)");
    BF_Test_Parse_Latex("(a<<<1)+(a<<<2)+(a<<<3)");
    BF_Test_Parse_Latex("(a*a*a*a*a) - (a*a*a)");
    BF_Test_Parse_Latex("(a*a)+(a*a*a)+(a*a*a*a)");
    BF_Test_Parse_Latex("(a+b)^(a+b)^(a+b)");
    BF_Test_Parse_Latex("((a+b)+(c+d))+((e+f)+(g+h))");

    BF_Test_Parse_Latex("a*a*a*a*a*a*a*a*a*a*a*a*a*a");
    BF_Test_Parse_Latex("(a+a+a+a)*(a+a+a+a)");
    BF_Test_Parse_Latex("((a*b)+(a*c))*((d*e)+(d*f))");
    BF_Test_Parse_Latex("((a^b)^c)^((d^e)^f)");
    BF_Test_Parse_Latex("(a**2)*(a**3)*(a**4)");
    BF_Test_Parse_Latex("((a<<<1)<<<2)<<<3");
    BF_Test_Parse_Latex("((a+b+c+d+e+f))");
    BF_Test_Parse_Latex("(a*a*a*a*a*a)/(a*a*a)");
    BF_Test_Parse_Latex("(a+b)*(c+d)*(e+f)");
    BF_Test_Parse_Latex("(a^a^a^a^a)");
    return 0;
}

int main() {
    BF_RUN_TEST(Test_Fuzzing);
    return 0;
}