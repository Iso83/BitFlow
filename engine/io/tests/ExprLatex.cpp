#include "TestAssert.h"
#include "common/Expr.h"
#include "common/Rule.h"

#include <BitFlow/engine/core/rules/RulePipeline.h>
#include <BitFlow/engine/io/ExprLatex.h>
#include <BitFlow/engine/io/ExprParser.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;
using namespace BitFlow::Engine::IO;

inline std::string ToLatex(const ParseResult& result) {
    return ToLatex(result.root, result.names);
}

int TestExprLatex_Const() {
    MakeExprStore(32);
    CPPTEST_ASSERT(ToLatex(C(42)) == "42");
    return 0;
}

int TestExprLatex_AddMulPrecedence() {
    MakeExprStore(32);
    CPPTEST_ASSERT(ToLatex(Parse("(a + b) * c")) == "(a + b) \\cdot c");
    return 0;
}

int TestExprLatex_ShiftAndAdd() {
    MakeExprStore(32);
    CPPTEST_ASSERT(ToLatex(Parse("x << (y + 3)")) == "x \\ll (y + 3)");
    return 0;
}

int TestExprLatex_UnaryNot() {
    MakeExprStore(32);
    CPPTEST_ASSERT(ToLatex(Parse("~a")) == "\\sim a");
    return 0;
}

int TestExprLatex_Xor() {
    MakeExprStore(32);
    CPPTEST_ASSERT(ToLatex(Parse("a ^ b")) == "a \\oplus b");
    return 0;
}

int TestExprLatex_Rotl() {
    MakeExprStore(32);
    CPPTEST_ASSERT(ToLatex(Parse("(a + b) <<< 3")) == "\\operatorname{rotl}(a + b, 3)");
    return 0;
}

int TestExprLatex_RoundTripStructure() {
    MakeExprStore(32);
    CPPTEST_ASSERT(!ToLatex(Parse("~a ^ (b + 3) << 2")).empty());
    return 0;
}

int TestExprLatex_Pow() {
    MakeExprStore(32);
    CPPTEST_ASSERT(ToLatex(Parse("(a + b) ** (b + 3)")) == "(a + b)^{b + 3}");
    return 0;
}

int TestExprLatex_Sub() {
    MakeExprStore(32);
    CPPTEST_ASSERT(ToLatex(Parse("a - b")) == "a - b");
    return 0;
}

int TestExprLatex_DivFraction() {
    MakeExprStore(32);
    CPPTEST_ASSERT(ToLatex(Parse("(a + b) / 3")) == "\\frac{a + b}{3}");
    return 0;
}

int TestExprLatex_Mod() {
    MakeExprStore(32);
    CPPTEST_ASSERT(ToLatex(Parse("a % b")) == "a \\bmod b");
    return 0;
}

int TestExprLatex_And() {
    MakeExprStore(32);
    CPPTEST_ASSERT(ToLatex(Parse("a & b")) == "a \\land b");
    return 0;
}

int TestExprLatex_Or() {
    MakeExprStore(32);
    CPPTEST_ASSERT(ToLatex(Parse("a | b")) == "a \\lor b");
    return 0;
}

int TestExprLatex_Neg() {
    MakeExprStore(32);
    CPPTEST_ASSERT(ToLatex(Parse("-a")) == "-a");
    return 0;
}

int TestExprLatex_Shr() {
    MakeExprStore(32);
    CPPTEST_ASSERT(ToLatex(Parse("a >> 3")) == "a \\gg 3");
    return 0;
}

int TestExprLatex_RotR() {
    MakeExprStore(32);
    CPPTEST_ASSERT(ToLatex(Parse("a >>> 3")) == "\\operatorname{rotr}(a, 3)");
    return 0;
}

int TestExprLatex_DivGrouping() {
    MakeExprStore(32);

    CPPTEST_ASSERT(ToLatex(Parse("(a + b) / (c + d)")) == "\\frac{a + b}{c + d}");

    return 0;
}

int TestExprLatex_PowGrouping() {
    MakeExprStore(32);

    CPPTEST_ASSERT(ToLatex(Parse("(a + b) ** (c + d)")) == "(a + b)^{c + d}");

    return 0;
}

int TestExprLatex_UnaryPow() {
    MakeExprStore(32);

    CPPTEST_ASSERT(ToLatex(Parse("~(a ^ b)")) == "\\sim (a \\oplus b)");

    return 0;
}

int TestExprLatex_MulDivPrecedence() {
    MakeExprStore(32);

    CPPTEST_ASSERT(ToLatex(Parse("a * (b / c)")) == "a \\cdot \\frac{b}{c}");

    return 0;
}

int TestExprLatex_PowPowAssociativity() {
    MakeExprStore(32);

    CPPTEST_ASSERT(ToLatex(Parse("a ** b ** c")) == "a^{b^{c}}");

    return 0;
}

int TestExprLatex_NegGrouping() {
    MakeExprStore(32);

    CPPTEST_ASSERT(ToLatex(Parse("-(a + b)")) == "-(a + b)");

    return 0;
}

int TestExprLatex_NotGrouping() {
    MakeExprStore(32);

    CPPTEST_ASSERT(ToLatex(Parse("~(a + b)")) == "\\sim (a + b)");

    return 0;
}

int TestExprLatex_FractionInsidePow() {
    MakeExprStore(32);

    CPPTEST_ASSERT(ToLatex(Parse("((a + b) / c) ** 2")) == "\\left(\\frac{a + b}{c}\\right)^{2}");

    return 0;
}

int TestExprLatex_PowMulPrecedence() {
    MakeExprStore(32);

    CPPTEST_ASSERT(ToLatex(Parse("a * b ** 2")) == "a \\cdot b^{2}");

    return 0;
}

int TestExprLatex_MultiAdd() {
    MakeExprStore(32);

    CPPTEST_ASSERT(ToLatex(Parse("a + b + c")) == "a + b + c");

    return 0;
}

int TestExprLatex_MultiMul() {
    MakeExprStore(32);

    CPPTEST_ASSERT(ToLatex(Parse("a * b * c")) == "a \\cdot b \\cdot c");

    return 0;
}

int TestExprLatex_ComplexPrecedence() {
    MakeExprStore(32);

    CPPTEST_ASSERT(ToLatex(Parse("~(a + b) * c ** 2")) == "\\sim (a + b) \\cdot c^{2}");

    return 0;
}

int TestExprLatex_RotlGrouping() {
    MakeExprStore(32);

    CPPTEST_ASSERT(ToLatex(Parse("(a + b) <<< (c + d)")) == "\\operatorname{rotl}(a + b, c + d)");

    return 0;
}

int TestExprLatex_RewrittenNaryXor() {
    MakeExprStore(32);

    RuleEngine engine = BuildExplore();
    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE(a ^ b ^ (a + C(8))));

    CPPTEST_ASSERT(ToLatex(r, names) == "a \\oplus b \\oplus (8 + a)");

    return 0;
}

int TestExprLatex_RewrittenMixedBitwiseArithmetic() {
    MakeExprStore(32);

    RuleEngine engine = BuildExplore();
    auto parsed = Parse("a^b+7|a+8*4-(f+f+f)-f**2");

    BF_SAFE_REWRITE(r, BF_REWRITE(parsed.root));

    CPPTEST_ASSERT(!ToLatex(r, parsed.names).empty());

    return 0;
}

int main() {
    CPPTEST_RUN(TestExprLatex_Const);
    CPPTEST_RUN(TestExprLatex_AddMulPrecedence);
    CPPTEST_RUN(TestExprLatex_ShiftAndAdd);
    CPPTEST_RUN(TestExprLatex_UnaryNot);
    CPPTEST_RUN(TestExprLatex_Xor);
    CPPTEST_RUN(TestExprLatex_Rotl);
    CPPTEST_RUN(TestExprLatex_RoundTripStructure);
    CPPTEST_RUN(TestExprLatex_Pow);

    CPPTEST_RUN(TestExprLatex_Sub);
    CPPTEST_RUN(TestExprLatex_DivFraction);
    CPPTEST_RUN(TestExprLatex_Mod);
    CPPTEST_RUN(TestExprLatex_And);
    CPPTEST_RUN(TestExprLatex_Or);
    CPPTEST_RUN(TestExprLatex_Neg);
    CPPTEST_RUN(TestExprLatex_Shr);
    CPPTEST_RUN(TestExprLatex_RotR);
    CPPTEST_RUN(TestExprLatex_DivGrouping);
    CPPTEST_RUN(TestExprLatex_PowGrouping);
    CPPTEST_RUN(TestExprLatex_UnaryPow);

    CPPTEST_RUN(TestExprLatex_MulDivPrecedence);
    CPPTEST_RUN(TestExprLatex_PowPowAssociativity);
    CPPTEST_RUN(TestExprLatex_NegGrouping);
    CPPTEST_RUN(TestExprLatex_NotGrouping);
    CPPTEST_RUN(TestExprLatex_FractionInsidePow);

    CPPTEST_RUN(TestExprLatex_PowMulPrecedence);
    CPPTEST_RUN(TestExprLatex_MultiAdd);
    CPPTEST_RUN(TestExprLatex_MultiMul);

    CPPTEST_RUN(TestExprLatex_ComplexPrecedence);
    CPPTEST_RUN(TestExprLatex_RotlGrouping);
    CPPTEST_RUN(TestExprLatex_RewrittenNaryXor);
    CPPTEST_RUN(TestExprLatex_RewrittenMixedBitwiseArithmetic);

    return 0;
}
