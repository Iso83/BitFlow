#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <map>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Testing;

int main() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);

    auto addExpr = MakeOp(10, OpType::Add, {a, b});
    auto addCode = Codegen::EmitCExpr(addExpr, 32);

    BF_TEST(addCode.find("+") != std::string::npos);
    BF_TEST(addCode.find("((1ull << 32) - 1ull)") != std::string::npos);

    auto c1 = MakeConst(11, 1);
    auto c40 = MakeConst(12, 40);
    auto shlExpr = MakeOp(13, OpType::Shl, {c1, c40});

    auto shlEval = Eval::EvaluateConstant(shlExpr, 32);
    BF_TEST(shlEval.status == Eval::EvalStatus::Success);
    BF_TEST(shlEval.value == (1ull << (40ull % 32ull)));

    auto shlCode = Codegen::EmitCExpr(shlExpr, 32);
    BF_TEST(shlCode.find("% 32ull") != std::string::npos);

    auto rotExpr = MakeOp(14, OpType::RotL, {MakeConst(15, 0x12u), MakeConst(16, 33u)});
    auto rotEval = Eval::EvaluateConstant(rotExpr, 32);
    BF_TEST(rotEval.status == Eval::EvalStatus::Success);

    auto rotCode = Codegen::EmitCExpr(rotExpr, 32);
    BF_TEST(rotCode.find("% 32ull") != std::string::npos);
    BF_TEST(rotCode.find("((1ull << 32) - 1ull)") != std::string::npos);

    // Case 1 — precedence add/mul: (a + b) * c
    auto mulOverAddExpr = MakeOp(17, OpType::Mul, {MakeOp(18, OpType::Add, {a, b}), c});
    auto mulOverAddCode = Codegen::EmitCExpr(mulOverAddExpr, 32);
    BF_TEST(mulOverAddCode ==
            "(((((((((v1) & ((1ull << 32) - 1ull)) + ((v2) & ((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull))) * "
            "((v3) & ((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull))");

    // Case 2 — shift met add rechts: a << (b + c)
    auto shlAddExpr = MakeOp(19, OpType::Shl, {a, MakeOp(20, OpType::Add, {b, c})});
    auto shlAddCode = Codegen::EmitCExpr(shlAddExpr, 32);
    BF_TEST(shlAddCode ==
            "((((((v1) & ((1ull << 32) - 1ull)) << ((((((v2) & ((1ull << 32) - 1ull)) + ((v3) & ((1ull << 32) - "
            "1ull))) & ((1ull << 32) - 1ull))) % 32ull)) & ((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull))");

    // Case 3 — unary boven binary: ~(a ^ b)
    auto notXorExpr = MakeOp(21, OpType::Not, {MakeOp(22, OpType::Xor, {a, b})});
    auto notXorCode = Codegen::EmitCExpr(notXorExpr, 32);
    BF_TEST(notXorCode ==
            "((((~(((((v1) & ((1ull << 32) - 1ull)) ^ ((v2) & ((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull)))) & "
            "((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull))");

    // Case 4 — nested bitwise: (a ^ b) & c
    auto andOverXorExpr = MakeOp(23, OpType::And, {MakeOp(24, OpType::Xor, {a, b}), c});
    auto andOverXorCode = Codegen::EmitCExpr(andOverXorExpr, 32);
    BF_TEST(andOverXorCode ==
            "(((((((((v1) & ((1ull << 32) - 1ull)) ^ ((v2) & ((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull))) & "
            "((v3) & ((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull))) & ((1ull << 32) - 1ull))");

    // Case 5 — canonical mask vorm (32/64 bit)
    BF_TEST(Codegen::EmitCExpr(a, 32).find("((1ull << 32) - 1ull)") != std::string::npos);
    BF_TEST(Codegen::EmitCExpr(a, 64) == "((((v1) & 0xffffffffffffffffull)) & 0xffffffffffffffffull)");

    // Case 6 — variabele mapping (id -> naam)
    const std::map<uint32_t, std::string> names = {{1u, "lhs"}, {2u, "rhs"}};
    const auto resolved = Codegen::BuildVarNameMap(addExpr, names);
    BF_TEST(resolved.at(1u) == "lhs");
    BF_TEST(resolved.at(2u) == "rhs");

    // Case 6b — alle Var nodes verzamelen, zonder duplicaten
    auto duplicateVarExpr = MakeOp(25, OpType::Add, {a, MakeOp(26, OpType::Xor, {a, b})});
    const auto resolvedDup = Codegen::BuildVarNameMap(duplicateVarExpr);
    BF_TEST(resolvedDup.size() == 2);
    BF_TEST(resolvedDup.count(1u) == 1u);
    BF_TEST(resolvedDup.count(2u) == 1u);
    BF_TEST(Codegen::BuildVarNameMap(c).at(3u) == "v3");

    // Case 7 — parameterlijst genereren
    const auto params = Codegen::EmitCParamList(addExpr, 32, names);
    BF_TEST(params == "uint64_t lhs, uint64_t rhs");

    // Case 7b — deterministische oplopende volgorde
    auto unorderedVars = MakeOp(27, OpType::Add, {MakeVar(9), MakeOp(28, OpType::Xor, {MakeVar(2), MakeVar(5)})});
    const auto orderedParams = Codegen::EmitCParamList(unorderedVars, 32);
    BF_TEST(orderedParams == "uint64_t v2, uint64_t v5, uint64_t v9");

    // Case 8 — function wrapper genereren
    const auto fn = Codegen::EmitCFunction(addExpr, 32, "bf_eval_add", names);
    BF_TEST(fn.find("uint64_t bf_eval_add(uint64_t lhs, uint64_t rhs)") != std::string::npos);
    BF_TEST(fn.find("return ") != std::string::npos);
    BF_TEST(fn.find("lhs") != std::string::npos);
    BF_TEST(fn.find("rhs") != std::string::npos);

    // Case 9 — nieuwe API overload met default functienaam
    const auto defaultFn = Codegen::EmitCFunction(addExpr, 32);
    BF_TEST(defaultFn.find("uint64_t f(uint64_t v1, uint64_t v2)") != std::string::npos);
    BF_TEST(defaultFn.find("return ") != std::string::npos);

    // Case 9b — signature gebruikt sorted ids zonder duplicaten
    const auto dedupFn = Codegen::EmitCFunction(duplicateVarExpr, 32);
    BF_TEST(dedupFn.find("uint64_t f(uint64_t v1, uint64_t v2)") != std::string::npos);

    // Case 10 — body gebruikt bestaande emitter output
    const auto exprBody = Codegen::EmitCExpr(addExpr, 32);
    BF_TEST(defaultFn.find("return " + exprBody + ";") != std::string::npos);

    return 0;
}
