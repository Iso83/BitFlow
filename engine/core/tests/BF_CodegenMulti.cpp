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
    // Case 1 — twee outputs zonder sharing
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);
    auto d = MakeVar(4);
    auto out1 = MakeOp(10, OpType::Add, {a, b});
    auto out2 = MakeOp(11, OpType::Xor, {c, d});
    const std::vector<const AST::Expr*> noSharing = {out1, out2};
    const auto noSharingCode = Codegen::EmitCFunctionMulti(noSharing, 32);
    BF_TEST(noSharingCode.find("out1") != std::string::npos);
    BF_TEST(noSharingCode.find("out2") != std::string::npos);

    // Case 2 — gedeelde subexpressie
    auto shared = MakeOp(20, OpType::Xor, {a, b});
    auto sharedOut1 = MakeOp(21, OpType::Add, {shared, c});
    auto sharedOut2 = MakeOp(22, OpType::And, {shared, d});
    const std::vector<const AST::Expr*> sharedOutputs = {sharedOut1, sharedOut2};
    const auto sharedCode = Codegen::EmitCFunctionMulti(sharedOutputs, 32);
    BF_TEST(sharedCode.find("uint32_t t1 = ") != std::string::npos);
    BF_TEST(CountSubstring(sharedCode, "^") == 1u);
    BF_TEST(sharedCode.find("r.out1 = ") != std::string::npos);
    BF_TEST(sharedCode.find("r.out2 = ") != std::string::npos);
    BF_TEST(CountSubstring(sharedCode, "t1") >= 3u); // declaratie + gebruik in beide outputs

    // Case 3 — leafs mogen geen temp worden
    auto leafVar = MakeVar(30);
    auto leafConst = MakeConst(31, 7);
    const std::vector<const AST::Expr*> leafOutputs = {leafVar, leafConst};
    const auto leafCode = Codegen::EmitCFunctionMulti(leafOutputs, 32);
    BF_TEST(leafCode.find("uint32_t t1 = v30;") == std::string::npos);
    BF_TEST(leafCode.find("uint32_t t1 = 7ull;") == std::string::npos);
    BF_TEST(leafCode.find("uint32_t t1 = ") == std::string::npos);

    // Case 4 — nested shared nodes
    auto s1 = MakeOp(40, OpType::Xor, {a, b});
    auto s2 = MakeOp(41, OpType::Add, {s1, c});
    auto nestedOut1 = MakeOp(42, OpType::Mul, {s2, d});
    auto nestedOut2 = MakeOp(43, OpType::Add, {s2, s1});
    const std::vector<const AST::Expr*> nestedOutputs = {nestedOut1, nestedOut2};
    const auto nestedCode = Codegen::EmitCFunctionMulti(nestedOutputs, 32);
    const auto t1Pos = nestedCode.find("uint32_t t1 = ");
    const auto t2Pos = nestedCode.find("uint32_t t2 = ");
    const auto out1Pos = nestedCode.find("r.out1 = ");
    BF_TEST(t1Pos != std::string::npos);
    BF_TEST(t2Pos != std::string::npos);
    BF_TEST(out1Pos != std::string::npos);
    BF_TEST(t1Pos < t2Pos);
    BF_TEST(t2Pos < out1Pos);

    // Case 5 — wide outputs must initialize struct without default bf_uint ctor.
    const auto wideCode = Codegen::EmitCFunctionMulti(nestedOutputs, 128);
    BF_TEST(wideCode.find("EvalResult r{bf_uint(0ull, 128), bf_uint(0ull, 128)};") != std::string::npos);

    return 0;
}
