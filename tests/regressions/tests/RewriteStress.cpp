#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

#include <BitFlow/engine/core/rules/RulePipeline.h>
#include <BitFlow/engine/io/ExprLatex.h>
#include <BitFlow/engine/io/ExprParser.h>
#include <BitFlow/engine/io/ExprPrinter.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Rules;
using namespace BitFlow::Engine::IO;

static int Run_Parse_Latex(const std::string& input) {
    MakeExprStore(32);

    RuleEngine engine = BuildExplore();

    auto parse = Parse(input);

    BF_SAFE_REWRITE(r, BF_REWRITE(parse.root, &parse.names));

    const auto text = BitFlow::Engine::IO::ToString(r, parse.names);
    CPPTEST_ASSERT(!text.empty());

    auto parse2 = Parse(text);

    BF_SAFE_REWRITE(r2, BF_REWRITE(parse2.root, &parse2.names));

    CPPTEST_ASSERT(!ToLatex(r2, parse2.names).empty());

    return 0;
}

#define BF_Test_Parse_Latex(input) CPPTEST_ASSERT(Run_Parse_Latex(input) == 0)

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
    CPPTEST_RUN(Test_Fuzzing);
    return 0;
}
