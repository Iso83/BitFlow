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

    BF_TEST(text == "((a & b) ^ ~(c))");
    return 0;
}

int TestExprPrinter_FromParserNames() {
    auto parsed = BitFlow::IO::Parse("a ^ b & c");
    auto text = BitFlow::IO::ToString(parsed.root, parsed.idToName);

    BF_TEST(text == "(a ^ (b & c))");
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

int main() {
    BF_RUN_TEST(TestExprPrinter_WithCustomNames);
    BF_RUN_TEST(TestExprPrinter_FromParserNames);
    BF_RUN_TEST(TestExprPrinter_Roundtrip_Bitwise);
    BF_RUN_TEST(TestExprPrinter_Roundtrip_ArithmeticShiftAndCalls);
    return 0;
}
