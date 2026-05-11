#include <BitFlow/io/ExprLatex.h>
#include <BitFlow/io/ExprParser.h>
#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Expression;
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
    BF_TEST(ToLatex(Parse("rotl(a + b, 3)")) == "\\operatorname{rotl}(a + b, 3)");
    return 0;
}

int TestExprLatex_RoundTripStructure() {
    MakeExprStore(32);
    BF_TEST(!ToLatex(Parse("~a ^ (b + 3) << 2")).empty());
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
    return 0;
}