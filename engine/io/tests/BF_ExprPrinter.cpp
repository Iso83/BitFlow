#include <BitFlow/io/ExprParser.h>
#include <BitFlow/io/ExprPrinter.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <unordered_map>

using namespace BitFlow::Core::Testing;

int TestExprPrinter_WithCustomNames() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);

    auto andExpr = MakeOp(10, OpType::And, {a, b});
    auto notExpr = MakeOp(11, OpType::Not, {c});
    auto expr = MakeOp(12, OpType::Xor, {andExpr, notExpr});

    std::unordered_map<uint32_t, std::string> names{{1, "a"}, {2, "b"}, {3, "c"}};
    auto text = BitFlow::IO::ToString(expr, names);

    BF_TEST(text == "a & b ^ ~c");
    return 0;
}

int TestExprPrinter_FromParserNames() {
    auto parsed = BitFlow::IO::Parse("a ^ b & c");
    auto text = BitFlow::IO::ToString(parsed.root, parsed.idToName);

    BF_TEST(text == "a ^ b & c");
    return 0;
}

int TestExprPrinter_Roundtrip_Bitwise() {
    auto parsedA = BitFlow::IO::Parse("a ^ b & ~c");
    const auto printedA = BitFlow::IO::ToString(parsedA.root, parsedA.idToName);

    auto parsedB = BitFlow::IO::Parse(printedA);
    const auto printedB = BitFlow::IO::ToString(parsedB.root, parsedB.idToName);

    BF_TEST(printedA == printedB);
    return 0;
}

int TestExprPrinter_Roundtrip_ArithmeticShiftAndCalls() {
    auto parsedA = BitFlow::IO::Parse("rotl(a + b, 3) ^ rotr(c << 1, 7)");
    const auto printedA = BitFlow::IO::ToString(parsedA.root, parsedA.idToName);

    auto parsedB = BitFlow::IO::Parse(printedA);
    const auto printedB = BitFlow::IO::ToString(parsedB.root, parsedB.idToName);

    BF_TEST(printedA == printedB);
    return 0;
}

int TestExprPrinter_RotateOutputStyleOption() {
    auto parsed = BitFlow::IO::Parse("rotl(a + b, 3) ^ rotr(c, 7)");

    BitFlow::IO::PrintOptions functionStyle{};
    functionStyle.rotAsFunction = true;
    BF_TEST(BitFlow::IO::ToString(parsed.root, parsed.idToName, functionStyle) == "rotl(a + b, 3) ^ rotr(c, 7)");

    BitFlow::IO::PrintOptions symbolicStyle{};
    symbolicStyle.rotAsFunction = false;
    BF_TEST(BitFlow::IO::ToString(parsed.root, parsed.idToName, symbolicStyle) == "a + b <<< 3 ^ c >>> 7");
    return 0;
}

int TestExprPrinter_SupportsShaCalls() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);

    auto chExpr = MakeOp(10, OpType::Ch, {a, b, c});
    auto majExpr = MakeOp(11, OpType::Maj, {a, b, c});

    std::unordered_map<uint32_t, std::string> names{{1, "a"}, {2, "b"}, {3, "c"}};
    BF_TEST(BitFlow::IO::ToString(chExpr, names) == "ch(a, b, c)");
    BF_TEST(BitFlow::IO::ToString(majExpr, names) == "maj(a, b, c)");
    return 0;
}

int TestExprPrinter_PrecedenceAwareParentheses() {
    auto parsedA = BitFlow::IO::Parse("a + (b - c)");
    BF_TEST(BitFlow::IO::ToString(parsedA.root, parsedA.idToName) == "a + (b - c)");

    auto parsedB = BitFlow::IO::Parse("a - (b + c)");
    BF_TEST(BitFlow::IO::ToString(parsedB.root, parsedB.idToName) == "a - (b + c)");

    auto parsedC = BitFlow::IO::Parse("(a + b) - c");
    BF_TEST(BitFlow::IO::ToString(parsedC.root, parsedC.idToName) == "a + b - c");

    return 0;
}

    auto parsedF = BitFlow::IO::Parse("~(a & b)");
    BF_TEST(BitFlow::IO::ToString(parsedF.root, parsedF.idToName) == "~(a & b)");

    auto parsedG = BitFlow::IO::Parse("-(-a)");
    BF_TEST(BitFlow::IO::ToString(parsedG.root, parsedG.idToName) == "--a");

int TestExprPrinter_Roundtrip_ParsePrintParse() {
    auto parsedA = BitFlow::IO::Parse("rotl(a + b, 3) ^ rotr(c << 1, 7) ^ (x & y) ^ (~z)");
    const auto printedA = BitFlow::IO::ToString(parsedA.root, parsedA.idToName);

    auto parsedB = BitFlow::IO::Parse(printedA);
    const auto printedB = BitFlow::IO::ToString(parsedB.root, parsedB.idToName);

    BF_TEST(printedA == printedB);
    return 0;
}

int TestExprPrinter_PrecedenceSensitivePrint() {
    auto parsedA = BitFlow::IO::Parse("a ^ b & c");
    BF_TEST(BitFlow::IO::ToString(parsedA.root, parsedA.idToName) == "a ^ b & c");

    auto parsedB = BitFlow::IO::Parse("a - (b + c)");
    BF_TEST(BitFlow::IO::ToString(parsedB.root, parsedB.idToName) == "a - (b + c)");

    auto parsedC = BitFlow::IO::Parse("(a + b) - c");
    BF_TEST(BitFlow::IO::ToString(parsedC.root, parsedC.idToName) == "a + b - c");

    return 0;
}

int main() {
    BF_RUN_TEST(TestExprPrinter_WithCustomNames);
    BF_RUN_TEST(TestExprPrinter_FromParserNames);
    BF_RUN_TEST(TestExprPrinter_Roundtrip_Bitwise);
    BF_RUN_TEST(TestExprPrinter_Roundtrip_ArithmeticShiftAndCalls);
    BF_RUN_TEST(TestExprPrinter_RotateOutputStyleOption);
    BF_RUN_TEST(TestExprPrinter_SupportsShaCalls);
    BF_RUN_TEST(TestExprPrinter_PrecedenceAwareParentheses);
    BF_RUN_TEST(TestExprPrinter_Roundtrip_ParsePrintParse);
    BF_RUN_TEST(TestExprPrinter_PrecedenceSensitivePrint);
    return 0;
}
