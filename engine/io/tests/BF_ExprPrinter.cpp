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

int main() {
    BF_RUN_TEST(TestExprPrinter_WithCustomNames);
    BF_RUN_TEST(TestExprPrinter_FromParserNames);
    return 0;
}
