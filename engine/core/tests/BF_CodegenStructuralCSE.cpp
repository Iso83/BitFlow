#include <BitFlow/core/codegen/Emitter.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <string>
#include <vector>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Testing;

namespace {

static size_t CountSubstring(const std::string& text, const std::string& needle) {
    if (needle.empty())
        return 0;

    size_t count = 0;
    size_t pos = 0;
    while ((pos = text.find(needle, pos)) != std::string::npos) {
        ++count;
        pos += needle.size();
    }
    return count;
}

} // namespace

int main() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);
    auto d = MakeVar(4);

    // Case 1 — identieke maar niet gedeelde subbomen
    auto x1 = MakeOp(100, OpType::Xor, {a, b});
    auto x2 = MakeOp(101, OpType::Xor, {a, b}); // apart Expr*, wel structureel gelijk
    auto out1 = MakeOp(102, OpType::Add, {x1, c});
    auto out2 = MakeOp(103, OpType::And, {x2, d});
    const std::vector<const AST::Expr*> outputs1 = {out1, out2};
    const auto code1 = Codegen::EmitCFunctionMulti(outputs1, 32);
    BF_TEST(code1.find("uint32_t t1 = ") != std::string::npos);
    BF_TEST(CountSubstring(code1, "^") == 1u);
    BF_TEST(code1.find("r.out1 = ") != std::string::npos);
    BF_TEST(code1.find("r.out2 = ") != std::string::npos);

    // Case 2 — pointer-sharing scenario blijft werken
    auto sharedXor = MakeOp(110, OpType::Xor, {a, b});
    auto sharedOut1 = MakeOp(111, OpType::Add, {sharedXor, c});
    auto sharedOut2 = MakeOp(112, OpType::And, {sharedXor, d});
    const std::vector<const AST::Expr*> outputs2 = {sharedOut1, sharedOut2};
    const auto code2 = Codegen::EmitCFunctionMulti(outputs2, 32);
    BF_TEST(code2.find("uint32_t t1 = ") != std::string::npos);
    BF_TEST(CountSubstring(code2, "^") == 1u);

    // Case 3 — verschillende volgorde mag NIET matchen
    auto xorAB = MakeOp(120, OpType::Xor, {a, b});
    auto xorBA = MakeOp(121, OpType::Xor, {b, a});
    const std::vector<const AST::Expr*> outputs3 = {xorAB, xorBA};
    const auto code3 = Codegen::EmitCFunctionMulti(outputs3, 32);
    BF_TEST(code3.find("uint32_t t1 = ") == std::string::npos);

    // Case 4 — unary exact match (~(a ^ b)) op aparte subbomen
    auto n1 = MakeOp(130, OpType::Not, {MakeOp(131, OpType::Xor, {a, b})});
    auto n2 = MakeOp(132, OpType::Not, {MakeOp(133, OpType::Xor, {a, b})});
    const std::vector<const AST::Expr*> outputs4 = {n1, n2};
    const auto code4 = Codegen::EmitCFunctionMulti(outputs4, 32);
    BF_TEST(code4.find("uint32_t t1 = ") != std::string::npos);
    BF_TEST(CountSubstring(code4, "~") == 1u);

    // Case 5 — nested exact match
    auto nested1 = MakeOp(140, OpType::Mul, {MakeOp(141, OpType::Add, {MakeOp(142, OpType::Xor, {a, b}), c}), d});
    auto nested2 = MakeOp(143, OpType::Mul, {MakeOp(144, OpType::Add, {MakeOp(145, OpType::Xor, {a, b}), c}), d});
    auto nestedOut1 = MakeOp(146, OpType::Add, {nested1, a});
    auto nestedOut2 = MakeOp(147, OpType::Sub, {nested2, b});
    const std::vector<const AST::Expr*> outputs5 = {nestedOut1, nestedOut2};
    const auto code5 = Codegen::EmitCFunctionMulti(outputs5, 32);
    BF_TEST(code5.find("uint32_t t1 = ") != std::string::npos);
    BF_TEST(CountSubstring(code5, "*") == 1u);

    return 0;
}
