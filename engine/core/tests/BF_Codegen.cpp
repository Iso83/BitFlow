#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Testing;

int main() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);

    // EmitCExpr uses canonical C expression output.
    auto addExpr = MakeOp(10, OpType::Add, {a, b});
    const auto addCode = Codegen::EmitCExpr(addExpr, 32);
    BF_TEST(addCode.find("v1") != std::string::npos);
    BF_TEST(addCode.find("v2") != std::string::npos);
    BF_TEST(addCode.find("+") != std::string::npos);
    BF_TEST(addCode.find("0xffffffffu") != std::string::npos);

    // Shift amount must be masked by bit width via SSA/perf-aware output.
    auto shlExpr = MakeOp(11, OpType::Shl, {MakeConst(12, 1), MakeConst(13, 40)});
    auto shlEval = Eval::EvaluateConstant(shlExpr, 32);
    BF_TEST(shlEval.status == Eval::EvalStatus::Success);
    const auto shlCode = Codegen::EmitCExpr(shlExpr, 32);
    BF_TEST(!shlCode.empty());

    // EmitCFunction signature and wrapper remain available.
    const auto fn = Codegen::EmitCFunction(addExpr, 32);
    BF_TEST(fn.find("#include <cstdint>") == std::string::npos);
    BF_TEST(fn.find("uint32_t eval(uint32_t v1, uint32_t v2)") != std::string::npos);
    BF_TEST(fn.find("uint32_t f(uint32_t v1, uint32_t v2)") != std::string::npos);
    BF_TEST(fn.find("return eval(v1, v2);") != std::string::npos);

    auto rotExpr32 = MakeOp(16, OpType::RotL, {a, MakeConst(17, 1)});
    const auto rotFn32 = Codegen::EmitCFunction(rotExpr32, 32);
    BF_TEST(rotFn32.find("bf_rotl(") != std::string::npos);

    // Shared subtree should produce temp in SSA-based function body.
    auto shared = MakeOp(20, OpType::Add, {a, b});
    auto expr = MakeOp(21, OpType::Mul, {shared, shared});
    const auto sharedFn = Codegen::EmitCFunction(expr, 32);
    BF_TEST(sharedFn.find("t0") != std::string::npos);

    // Wide bitwidth must route through bf_uint type.
    auto xorExpr = MakeOp(30, OpType::Xor, {a, b});
    const auto wideFn = Codegen::EmitCFunction(xorExpr, 128);
    BF_TEST(wideFn.find("bf_uint") != std::string::npos);
    BF_TEST(wideFn.find("#include <BitFlow/core/bitvector/BitVector.h>") == std::string::npos);

    const auto support32 = Codegen::EmitCRuntimeSupport(32);
    BF_TEST(support32.find("#include <cstdint>") != std::string::npos);
    BF_TEST(support32.find("bitWidth <= 32") != std::string::npos);
    BF_TEST(support32.find("bf_rotl(uint32_t") != std::string::npos);
    BF_TEST(support32.find("bf_rotr(uint32_t") != std::string::npos);

    const auto support64 = Codegen::EmitCRuntimeSupport(64);
    BF_TEST(support64.find("bf_rotl(uint64_t") != std::string::npos);
    BF_TEST(support64.find("bf_rotr(uint64_t") != std::string::npos);

    const auto support128 = Codegen::EmitCRuntimeSupport(128);
    BF_TEST(support128.find("#include <BitFlow/core/bitvector/BitVector.h>") != std::string::npos);
    BF_TEST(support128.find("using bf_uint = BitFlow::Core::BitVector::bf_uint;") != std::string::npos);
    BF_TEST(support128.find("bf_rotl(const bf_uint&") != std::string::npos);
    BF_TEST(support128.find("bf_rotr(const bf_uint&") != std::string::npos);

    // Unsigned literal suffixing should stay explicit in 64-bit mode.
    auto constExpr64 = MakeConst(31, 0xffffffffu);
    const auto constCode64 = Codegen::EmitCExpr(constExpr64, 64);
    BF_TEST(constCode64.find("0xffffffffull") != std::string::npos);

    // Large 32-bit constants should keep explicit unsigned suffixing too.
    auto constExpr32 = MakeConst(32, 0x80000000u);
    const auto constCode32 = Codegen::EmitCExpr(constExpr32, 32);
    BF_TEST(constCode32.find("0x80000000u") != std::string::npos);

    return 0;
}
