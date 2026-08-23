#include "common/Assert.h"
#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

#include <BitFlow/engine/core/rules/RulePipeline.h>
#include <BitFlow/engine/io/ExprLatex.h>
#include <BitFlow/engine/io/ExprParser.h>
#include <BitFlow/engine/io/ExprPrinter.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;
using namespace BitFlow::Engine::IO;

static void ValidateRoundtrip(ExprStore* store, const std::string& text) {
    auto parsed = Parse(store, text);

    const auto printed = BitFlow::Engine::IO::ToString(parsed.root, parsed.names);

    auto reparsed = Parse(store, printed);

    BF_ASSERT(BitFlow::Engine::IO::ToString(parsed.root, parsed.names) ==
              BitFlow::Engine::IO::ToString(reparsed.root, reparsed.names));
}

int Test_MulInsideOr_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "7 | 2 * f");

    auto parse = Parse("7 | 2 * f");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "7 | 2 * f");

    return 0;
}

int Test_OrInsideMul_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "(7 | 2) * f");

    auto parse = Parse("(7 | 2) * f");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "(7 | 2) * f");

    return 0;
}

int Test_AddInsideMul_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "(a + b) * c");

    auto parse = Parse("(a + b) * c");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "(a + b) * c");

    return 0;
}

int Test_MulInsideAdd_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a + b * c");

    auto parse = Parse("a + b * c");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "a + b * c");

    return 0;
}

int Test_SubRightAdd_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a - (b + c)");

    auto parse = Parse("a - (b + c)");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "a - (b + c)");

    return 0;
}

int Test_SubLeftAssociative_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a - b - c");

    auto parse = Parse("a - b - c");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "a - b - c");

    return 0;
}

int Test_ShiftRightAdd_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a << (b + c)");

    auto parse = Parse("a << (b + c)");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "a << (b + c)");

    return 0;
}

int Test_AddInsideShift_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a + b << c");

    auto parse = Parse("a + b << c");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "a + b << c");

    return 0;
}

int Test_AndInsideOr_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a | b & c");

    auto parse = Parse("a | b & c");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "a | b & c");

    return 0;
}

int Test_OrInsideAnd_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "(a | b) & c");

    auto parse = Parse("(a | b) & c");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "(a | b) & c");

    return 0;
}

int Test_XorInsideAnd_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "(a ^ b) & c");

    auto parse = Parse("(a ^ b) & c");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "(a ^ b) & c");

    return 0;
}

int Test_AndInsideXor_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a ^ b & c");

    auto parse = Parse("a ^ b & c");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "a ^ b & c");

    return 0;
}

int Test_UnaryNegInsideMul_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "-a * b");

    auto parse = Parse("-a * b");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "-a * b");

    return 0;
}

int Test_AddInsideUnaryNeg_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "-(a + b)");

    auto parse = Parse("-(a + b)");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "-(a + b)");

    return 0;
}

int Test_PowInsideMul_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a * b ** 2");

    auto parse = Parse("a * b ** 2");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "a * b ** 2");

    return 0;
}

int Test_MulInsidePow_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "(a * b) ** 2");

    auto parse = Parse("(a * b) ** 2");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "(a * b) ** 2");

    return 0;
}

int Test_RotateLeft_AddInsideRotate_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a <<< b + 1");

    auto parse = Parse("a <<< b + 1");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "a <<< b + 1");

    return 0;
}

int Test_RotateLeft_RotateInsideAdd_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "(a <<< b) + 1");

    auto parse = Parse("(a <<< b) + 1");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "(a <<< b) + 1");

    return 0;
}

int Test_RotateRight_AddInsideRotate_NoParenthesesNeeded() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "a >>> b + 1");

    auto parse = Parse("a >>> b + 1");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "a >>> b + 1");

    return 0;
}

int Test_RotateRight_RotateInsideAdd_RequiresParentheses() {
    MakeExprStore(32);

    ValidateRoundtrip(&store, "(a >>> b) + 1");

    auto parse = Parse("(a >>> b) + 1");

    CPPTEST_ASSERT(BitFlow::Engine::IO::ToString(parse.root, parse.names) == "(a >>> b) + 1");

    return 0;
}

int main() {
    CPPTEST_RUN(Test_MulInsideOr_NoParenthesesNeeded);
    CPPTEST_RUN(Test_OrInsideMul_RequiresParentheses);

    CPPTEST_RUN(Test_AddInsideMul_RequiresParentheses);
    CPPTEST_RUN(Test_MulInsideAdd_NoParenthesesNeeded);

    CPPTEST_RUN(Test_SubRightAdd_RequiresParentheses);
    CPPTEST_RUN(Test_SubLeftAssociative_NoParenthesesNeeded);

    CPPTEST_RUN(Test_ShiftRightAdd_RequiresParentheses);
    CPPTEST_RUN(Test_AddInsideShift_NoParenthesesNeeded);

    CPPTEST_RUN(Test_AndInsideOr_NoParenthesesNeeded);
    CPPTEST_RUN(Test_OrInsideAnd_RequiresParentheses);

    CPPTEST_RUN(Test_XorInsideAnd_RequiresParentheses);
    CPPTEST_RUN(Test_AndInsideXor_NoParenthesesNeeded);

    CPPTEST_RUN(Test_UnaryNegInsideMul_NoParenthesesNeeded);
    CPPTEST_RUN(Test_AddInsideUnaryNeg_RequiresParentheses);

    CPPTEST_RUN(Test_PowInsideMul_NoParenthesesNeeded);
    CPPTEST_RUN(Test_MulInsidePow_RequiresParentheses);

    CPPTEST_RUN(Test_RotateLeft_AddInsideRotate_NoParenthesesNeeded);
    CPPTEST_RUN(Test_RotateLeft_RotateInsideAdd_RequiresParentheses);
    CPPTEST_RUN(Test_RotateRight_AddInsideRotate_NoParenthesesNeeded);
    CPPTEST_RUN(Test_RotateRight_RotateInsideAdd_RequiresParentheses);

    return 0;
}
