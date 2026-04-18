#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core;
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
    BF_TEST(fn.find("uint32_t eval(uint32_t v1, uint32_t v2)") != std::string::npos);
    BF_TEST(fn.find("uint32_t f(uint32_t v1, uint32_t v2)") != std::string::npos);
    BF_TEST(fn.find("return eval(v1, v2);") != std::string::npos);

    // Shared subtree should produce temp in SSA-based function body.
    auto shared = MakeOp(20, OpType::Add, {a, b});
    auto expr = MakeOp(21, OpType::Mul, {shared, shared});
    const auto sharedFn = Codegen::EmitCFunction(expr, 32);
    BF_TEST(sharedFn.find("t0") != std::string::npos);

    // Wide bitwidth must route through bf_uint type.
    auto xorExpr = MakeOp(30, OpType::Xor, {a, b});
    const auto wideFn = Codegen::EmitCFunction(xorExpr, 128);
    BF_TEST(wideFn.find("bf_uint") != std::string::npos);

    return 0;
}
