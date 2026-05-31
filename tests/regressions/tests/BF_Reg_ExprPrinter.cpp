#include <BitFlow/core/rules/RulePipeline.h>
#include <BitFlow/io/ExprLatex.h>
#include <BitFlow/io/ExprParser.h>
#include <BitFlow/io/ExprPrinter.h>
#include <ExprTestUtils.h>
#include <RuleTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::IO;

static void ValidateRoundtrip(ExprStore* store, const std::string& text) {
    auto parsed = Parse(store, text);

    const auto printed = BitFlow::IO::ToString(parsed.root, parsed.names);

    auto reparsed = Parse(store, printed);

    BF_ASSERT(BitFlow::IO::ToString(parsed.root, parsed.names) == BitFlow::IO::ToString(reparsed.root, reparsed.names));
}

int Test_MulInsideOr_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "7 | 2 * f");

    auto parse = Parse("7 | 2 * f");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "7 | 2 * f");

    return 0;
}

int Test_OrInsideMul_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "(7 | 2) * f");

    auto parse = Parse("(7 | 2) * f");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "(7 | 2) * f");

    return 0;
}

int Test_AddInsideMul_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "(a + b) * c");

    auto parse = Parse("(a + b) * c");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "(a + b) * c");

    return 0;
}

int Test_MulInsideAdd_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a + b * c");

    auto parse = Parse("a + b * c");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "a + b * c");

    return 0;
}

int Test_SubRightAdd_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a - (b + c)");

    auto parse = Parse("a - (b + c)");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "a - (b + c)");

    return 0;
}

int Test_SubLeftAssociative_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a - b - c");

    auto parse = Parse("a - b - c");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "a - b - c");

    return 0;
}

int Test_ShiftRightAdd_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a << (b + c)");

    auto parse = Parse("a << (b + c)");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "a << (b + c)");

    return 0;
}

int Test_AddInsideShift_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a + b << c");

    auto parse = Parse("a + b << c");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "a + b << c");

    return 0;
}

int Test_AndInsideOr_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a | b & c");

    auto parse = Parse("a | b & c");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "a | b & c");

    return 0;
}

int Test_OrInsideAnd_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "(a | b) & c");

    auto parse = Parse("(a | b) & c");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "(a | b) & c");

    return 0;
}

int Test_XorInsideAnd_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "(a ^ b) & c");

    auto parse = Parse("(a ^ b) & c");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "(a ^ b) & c");

    return 0;
}

int Test_AndInsideXor_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a ^ b & c");

    auto parse = Parse("a ^ b & c");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "a ^ b & c");

    return 0;
}

int Test_UnaryNegInsideMul_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "-a * b");

    auto parse = Parse("-a * b");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "-a * b");

    return 0;
}

int Test_AddInsideUnaryNeg_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "-(a + b)");

    auto parse = Parse("-(a + b)");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "-(a + b)");

    return 0;
}

int Test_PowInsideMul_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a * b ** 2");

    auto parse = Parse("a * b ** 2");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "a * b ** 2");

    return 0;
}

int Test_MulInsidePow_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "(a * b) ** 2");

    auto parse = Parse("(a * b) ** 2");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "(a * b) ** 2");

    return 0;
}

int Test_RotateLeft_AddInsideRotate_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a <<< b + 1");

    auto parse = Parse("a <<< b + 1");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "a <<< b + 1");

    return 0;
}

int Test_RotateLeft_RotateInsideAdd_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "(a <<< b) + 1");

    auto parse = Parse("(a <<< b) + 1");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "(a <<< b) + 1");

    return 0;
}

int Test_RotateRight_AddInsideRotate_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a >>> b + 1");

    auto parse = Parse("a >>> b + 1");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "a >>> b + 1");

    return 0;
}

int Test_RotateRight_RotateInsideAdd_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "(a >>> b) + 1");

    auto parse = Parse("(a >>> b) + 1");

    BF_TEST(BitFlow::IO::ToString(parse.root, parse.names) == "(a >>> b) + 1");

    return 0;
}

int main() {
    BF_RUN_TEST(Test_MulInsideOr_NoParenthesesNeeded);
    BF_RUN_TEST(Test_OrInsideMul_RequiresParentheses);

    BF_RUN_TEST(Test_AddInsideMul_RequiresParentheses);
    BF_RUN_TEST(Test_MulInsideAdd_NoParenthesesNeeded);

    BF_RUN_TEST(Test_SubRightAdd_RequiresParentheses);
    BF_RUN_TEST(Test_SubLeftAssociative_NoParenthesesNeeded);

    BF_RUN_TEST(Test_ShiftRightAdd_RequiresParentheses);
    BF_RUN_TEST(Test_AddInsideShift_NoParenthesesNeeded);

    BF_RUN_TEST(Test_AndInsideOr_NoParenthesesNeeded);
    BF_RUN_TEST(Test_OrInsideAnd_RequiresParentheses);

    BF_RUN_TEST(Test_XorInsideAnd_RequiresParentheses);
    BF_RUN_TEST(Test_AndInsideXor_NoParenthesesNeeded);

    BF_RUN_TEST(Test_UnaryNegInsideMul_NoParenthesesNeeded);
    BF_RUN_TEST(Test_AddInsideUnaryNeg_RequiresParentheses);

    BF_RUN_TEST(Test_PowInsideMul_NoParenthesesNeeded);
    BF_RUN_TEST(Test_MulInsidePow_RequiresParentheses);

    BF_RUN_TEST(Test_RotateLeft_AddInsideRotate_NoParenthesesNeeded);
    BF_RUN_TEST(Test_RotateLeft_RotateInsideAdd_RequiresParentheses);
    BF_RUN_TEST(Test_RotateRight_AddInsideRotate_NoParenthesesNeeded);
    BF_RUN_TEST(Test_RotateRight_RotateInsideAdd_RequiresParentheses);

    return 0;
}