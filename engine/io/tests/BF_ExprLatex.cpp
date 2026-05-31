#include <BitFlow/core/rules/RulePipeline.h>
#include <BitFlow/io/ExprLatex.h>
#include <BitFlow/io/ExprParser.h>
#include <ExprTestUtils.h>
#include <RuleTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::IO;

inline std::string ToLatex(const ParseResult& result) {
    return ToLatex(result.root, result.names);
}

int TestExprLatex_Const() {
    MakeExprStore(32);
    BF_TEST(ToLatex(C(42)) == "42");
    return 0;
}

int TestExprLatex_AddMulPrecedence() {
    MakeExprStore(32);
    BF_TEST(ToLatex(Parse("(a + b) * c")) == "(a + b) \\cdot c");
    return 0;
}

int TestExprLatex_ShiftAndAdd() {
    MakeExprStore(32);
    BF_TEST(ToLatex(Parse("x << (y + 3)")) == "x \\ll (y + 3)");
    return 0;
}

int TestExprLatex_UnaryNot() {
    MakeExprStore(32);
    BF_TEST(ToLatex(Parse("~a")) == "\\sim a");
    return 0;
}

int TestExprLatex_Xor() {
    MakeExprStore(32);
    BF_TEST(ToLatex(Parse("a ^ b")) == "a \\oplus b");
    return 0;
}

int TestExprLatex_Rotl() {
    MakeExprStore(32);
    BF_TEST(ToLatex(Parse("(a + b) <<< 3")) == "\\operatorname{rotl}(a + b, 3)");
    return 0;
}

int TestExprLatex_RoundTripStructure() {
    MakeExprStore(32);
    BF_TEST(!ToLatex(Parse("~a ^ (b + 3) << 2")).empty());
    return 0;
}

int TestExprLatex_Pow() {
    MakeExprStore(32);
    BF_TEST(ToLatex(Parse("(a + b) ** (b + 3)")) == "(a + b)^{b + 3}");
    return 0;
}

int TestExprLatex_Sub() {
    MakeExprStore(32);
    BF_TEST(ToLatex(Parse("a - b")) == "a - b");
    return 0;
}

int TestExprLatex_DivFraction() {
    MakeExprStore(32);
    BF_TEST(ToLatex(Parse("(a + b) / 3")) == "\\frac{a + b}{3}");
    return 0;
}

int TestExprLatex_Mod() {
    MakeExprStore(32);
    BF_TEST(ToLatex(Parse("a % b")) == "a \\bmod b");
    return 0;
}

int TestExprLatex_And() {
    MakeExprStore(32);
    BF_TEST(ToLatex(Parse("a & b")) == "a \\land b");
    return 0;
}

int TestExprLatex_Or() {
    MakeExprStore(32);
    BF_TEST(ToLatex(Parse("a | b")) == "a \\lor b");
    return 0;
}

int TestExprLatex_Neg() {
    MakeExprStore(32);
    BF_TEST(ToLatex(Parse("-a")) == "-a");
    return 0;
}

int TestExprLatex_Shr() {
    MakeExprStore(32);
    BF_TEST(ToLatex(Parse("a >> 3")) == "a \\gg 3");
    return 0;
}

int TestExprLatex_RotR() {
    MakeExprStore(32);
    BF_TEST(ToLatex(Parse("a >>> 3")) == "\\operatorname{rotr}(a, 3)");
    return 0;
}

int TestExprLatex_DivGrouping() {
    MakeExprStore(32);

    BF_TEST(ToLatex(Parse("(a + b) / (c + d)")) == "\\frac{a + b}{c + d}");

    return 0;
}

int TestExprLatex_PowGrouping() {
    MakeExprStore(32);

    BF_TEST(ToLatex(Parse("(a + b) ** (c + d)")) == "(a + b)^{c + d}");

    return 0;
}

int TestExprLatex_UnaryPow() {
    MakeExprStore(32);

    BF_TEST(ToLatex(Parse("~(a ^ b)")) == "\\sim (a \\oplus b)");

    return 0;
}

int TestExprLatex_MulDivPrecedence() {
    MakeExprStore(32);

    BF_TEST(ToLatex(Parse("a * (b / c)")) == "a \\cdot \\frac{b}{c}");

    return 0;
}

int TestExprLatex_PowPowAssociativity() {
    MakeExprStore(32);

    BF_TEST(ToLatex(Parse("a ** b ** c")) == "a^{b^{c}}");

    return 0;
}

int TestExprLatex_NegGrouping() {
    MakeExprStore(32);

    BF_TEST(ToLatex(Parse("-(a + b)")) == "-(a + b)");

    return 0;
}

int TestExprLatex_NotGrouping() {
    MakeExprStore(32);

    BF_TEST(ToLatex(Parse("~(a + b)")) == "\\sim (a + b)");

    return 0;
}

int TestExprLatex_FractionInsidePow() {
    MakeExprStore(32);

    BF_TEST(ToLatex(Parse("((a + b) / c) ** 2")) == "\\left(\\frac{a + b}{c}\\right)^{2}");

    return 0;
}

int TestExprLatex_PowMulPrecedence() {
    MakeExprStore(32);

    BF_TEST(ToLatex(Parse("a * b ** 2")) == "a \\cdot b^{2}");

    return 0;
}

int TestExprLatex_MultiAdd() {
    MakeExprStore(32);

    BF_TEST(ToLatex(Parse("a + b + c")) == "a + b + c");

    return 0;
}

int TestExprLatex_MultiMul() {
    MakeExprStore(32);

    BF_TEST(ToLatex(Parse("a * b * c")) == "a \\cdot b \\cdot c");

    return 0;
}

int TestExprLatex_ComplexPrecedence() {
    MakeExprStore(32);

    BF_TEST(ToLatex(Parse("~(a + b) * c ** 2")) == "\\sim (a + b) \\cdot c^{2}");

    return 0;
}

int TestExprLatex_RotlGrouping() {
    MakeExprStore(32);

    BF_TEST(ToLatex(Parse("(a + b) <<< (c + d)")) == "\\operatorname{rotl}(a + b, c + d)");

    return 0;
}

int TestExprLatex_RewrittenNaryXor() {
    MakeExprStore(32);

    RuleEngine engine = BuildExplore();
    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE(a ^ b ^ (a + C(8))));

    BF_TEST(ToLatex(r, names) == "a \\oplus b \\oplus (8 + a)");

    return 0;
}

int TestExprLatex_RewrittenMixedBitwiseArithmetic() {
    MakeExprStore(32);

    RuleEngine engine = BuildExplore();
    auto parsed = Parse("a^b+7|a+8*4-(f+f+f)-f**2");

    BF_SAFE_REWRITE(r, BF_REWRITE(parsed.root));

    BF_TEST(!ToLatex(r, parsed.names).empty());

    return 0;
}

int main() {
    BF_RUN_TEST(TestExprLatex_Const);
    BF_RUN_TEST(TestExprLatex_AddMulPrecedence);
    BF_RUN_TEST(TestExprLatex_ShiftAndAdd);
    BF_RUN_TEST(TestExprLatex_UnaryNot);
    BF_RUN_TEST(TestExprLatex_Xor);
    BF_RUN_TEST(TestExprLatex_Rotl);
    BF_RUN_TEST(TestExprLatex_RoundTripStructure);
    BF_RUN_TEST(TestExprLatex_Pow);

    BF_RUN_TEST(TestExprLatex_Sub);
    BF_RUN_TEST(TestExprLatex_DivFraction);
    BF_RUN_TEST(TestExprLatex_Mod);
    BF_RUN_TEST(TestExprLatex_And);
    BF_RUN_TEST(TestExprLatex_Or);
    BF_RUN_TEST(TestExprLatex_Neg);
    BF_RUN_TEST(TestExprLatex_Shr);
    BF_RUN_TEST(TestExprLatex_RotR);
    BF_RUN_TEST(TestExprLatex_DivGrouping);
    BF_RUN_TEST(TestExprLatex_PowGrouping);
    BF_RUN_TEST(TestExprLatex_UnaryPow);

    BF_RUN_TEST(TestExprLatex_MulDivPrecedence);
    BF_RUN_TEST(TestExprLatex_PowPowAssociativity);
    BF_RUN_TEST(TestExprLatex_NegGrouping);
    BF_RUN_TEST(TestExprLatex_NotGrouping);
    BF_RUN_TEST(TestExprLatex_FractionInsidePow);

    BF_RUN_TEST(TestExprLatex_PowMulPrecedence);
    BF_RUN_TEST(TestExprLatex_MultiAdd);
    BF_RUN_TEST(TestExprLatex_MultiMul);

    BF_RUN_TEST(TestExprLatex_ComplexPrecedence);
    BF_RUN_TEST(TestExprLatex_RotlGrouping);
    BF_RUN_TEST(TestExprLatex_RewrittenNaryXor);
    BF_RUN_TEST(TestExprLatex_RewrittenMixedBitwiseArithmetic);

    return 0;
}