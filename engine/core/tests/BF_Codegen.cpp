#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <map>
#include <vector>

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
    BF_TEST(params == "uint32_t lhs, uint32_t rhs");

    // Case 7b — deterministische oplopende volgorde
    auto unorderedVars = MakeOp(27, OpType::Add, {MakeVar(9), MakeOp(28, OpType::Xor, {MakeVar(2), MakeVar(5)})});
    const auto orderedParams = Codegen::EmitCParamList(unorderedVars, 32);
    BF_TEST(orderedParams == "uint32_t v2, uint32_t v5, uint32_t v9");
    BF_TEST(Codegen::EmitCParamList(addExpr, 64, names) == "uint64_t lhs, uint64_t rhs");

    // Case 8 — function wrapper genereren
    const auto fn = Codegen::EmitCFunction(addExpr, 32, "bf_eval_add", names);
    BF_TEST(fn.find("uint32_t bf_eval_add(uint32_t lhs, uint32_t rhs)") != std::string::npos);
    BF_TEST(fn.find("return ") != std::string::npos);
    BF_TEST(fn.find("lhs") != std::string::npos);
    BF_TEST(fn.find("rhs") != std::string::npos);

    // Case 9 — nieuwe API overload met default functienaam
    const auto defaultFn = Codegen::EmitCFunction(addExpr, 32);
    BF_TEST(defaultFn.find("uint32_t eval(uint32_t v1, uint32_t v2)") != std::string::npos);
    BF_TEST(defaultFn.find("return ") != std::string::npos);

    // Case 9b — signature gebruikt sorted ids zonder duplicaten
    const auto dedupFn = Codegen::EmitCFunction(duplicateVarExpr, 32);
    BF_TEST(dedupFn.find("uint32_t eval(uint32_t v1, uint32_t v2)") != std::string::npos);

    // Case 10 — body gebruikt SSA-locals en maskeert de return verplicht
    BF_TEST(defaultFn.find("uint32_t t0 = ") != std::string::npos);
    BF_TEST(defaultFn.find("return (t0) & ((1ull << 32) - 1);") != std::string::npos);

    // Case 11 — gevraagde basis test voor EmitCFunction
    auto a2 = MakeVar(1);
    auto b2 = MakeVar(2);
    auto expr = MakeOp(29, OpType::Add, {a2, b2});
    auto code = Codegen::EmitCFunction(expr, 32);
    BF_TEST(code.find("uint32_t eval(") != std::string::npos);
    BF_TEST(code.find("v1") != std::string::npos);
    BF_TEST(code.find("v2") != std::string::npos);

    // Case 15.7 — SSA emitter basischeck: (a + b) * (a + b)
    auto a3 = MakeVar(1);
    auto b3 = MakeVar(2);
    auto add3 = MakeOp(45, OpType::Add, {a3, b3});
    auto expr3 = MakeOp(46, OpType::Mul, {add3, add3});
    auto code3 = Codegen::EmitCFunction(expr3, 32);
    BF_TEST(code3.find("uint32_t eval") != std::string::npos);
    BF_TEST(code3.find("t0") != std::string::npos);
    BF_TEST(code3.find("return") != std::string::npos);

    // Case 12.2 — meerdere outputs + gedeelde subexpressies + tijdelijke variabelen + statements
    auto shared = MakeOp(30, OpType::Add, {a, b});
    auto out0Expr = MakeOp(31, OpType::Mul, {shared, c});
    auto out1Expr = MakeOp(32, OpType::Xor, {shared, a});
    const std::vector<const AST::Expr*> outputs = {out0Expr, out1Expr};
    const auto multiFn = Codegen::EmitCFunction(outputs, 32, "bf_eval_multi");
    BF_TEST(multiFn.find("struct Outputs") != std::string::npos);
    BF_TEST(multiFn.find("Outputs bf_eval_multi(") != std::string::npos);
    BF_TEST(multiFn.find("Outputs r{};") != std::string::npos);
    BF_TEST(multiFn.find("uint32_t t1 = ") != std::string::npos);
    BF_TEST(multiFn.find("& ((1ull << 32) - 1ull)") != std::string::npos);
    BF_TEST(multiFn.find("r.out1 = ") != std::string::npos);
    BF_TEST(multiFn.find("r.out2 = ") != std::string::npos);

    // Case 12.3 — nieuwe API alias blijft non-breaking naast bestaande EmitCFunction APIs
    const auto multiFnAlias = Codegen::EmitCFunctionMulti(outputs, 32);
    BF_TEST(multiFnAlias.find("Outputs f(") != std::string::npos);
    BF_TEST(multiFnAlias.find("r.out1 = ") != std::string::npos);
    BF_TEST(multiFnAlias.find("r.out2 = ") != std::string::npos);

    // Case 12.4 — structurele gelijkheid telt nu ook voor CSE/temp-hergebruik
    auto addLeft = MakeOp(33, OpType::Add, {a, b});
    auto addRight = MakeOp(34, OpType::Add, {a, b}); // structureel gelijk, maar andere Expr*
    const std::vector<const AST::Expr*> nonSharedOutputs = {addLeft, addRight};
    const auto nonSharedFn = Codegen::EmitCFunctionMulti(nonSharedOutputs, 32);
    BF_TEST(nonSharedFn.find("uint32_t t1 = ") != std::string::npos);

    // Case 12.5 — zelfde vorm maar andere inhoud is NIET structureel gelijk
    auto addDifferent = MakeOp(38, OpType::Add, {a, c});
    const std::vector<const AST::Expr*> shapeOnlyOutputs = {addLeft, addDifferent};
    const auto shapeOnlyFn = Codegen::EmitCFunctionMulti(shapeOnlyOutputs, 32);
    BF_TEST(shapeOnlyFn.find("uint32_t t1 = ") == std::string::npos);

    // Case 12.6 — post-order/temp statement order (children voor parent)
    auto sharedChild = MakeOp(35, OpType::Add, {a, b});
    auto sharedParent = MakeOp(36, OpType::Mul, {sharedChild, c});
    auto out2Expr = MakeOp(37, OpType::Add, {sharedParent, sharedChild});
    const std::vector<const AST::Expr*> orderedOutputs = {sharedParent, out2Expr};
    const auto orderedFn = Codegen::EmitCFunctionMulti(orderedOutputs, 32);
    const auto t1Pos = orderedFn.find("uint32_t t1 = ");
    const auto t2Pos = orderedFn.find("uint32_t t2 = ");
    const auto out1Pos = orderedFn.find("r.out1 = ");
    BF_TEST(t1Pos != std::string::npos);
    BF_TEST(t2Pos != std::string::npos);
    BF_TEST(out1Pos != std::string::npos);
    BF_TEST(t1Pos < t2Pos);
    BF_TEST(t2Pos < out1Pos);

    // Case 13.3a — geen commutativiteit: (a ^ b) != (b ^ a)
    auto xorAB = MakeOp(39, OpType::Xor, {a, b});
    auto xorBA = MakeOp(40, OpType::Xor, {b, a});
    const std::vector<const AST::Expr*> nonCommutativeOutputs = {xorAB, xorBA};
    const auto nonCommutativeFn = Codegen::EmitCFunctionMulti(nonCommutativeOutputs, 32);
    BF_TEST(nonCommutativeFn.find("uint32_t t1 = ") == std::string::npos);

    // Case 13.3b — geen associativiteit: a + (b + c) != (a + b) + c
    auto addRightNested = MakeOp(41, OpType::Add, {a, MakeOp(42, OpType::Add, {b, c})});
    auto addLeftNested = MakeOp(43, OpType::Add, {MakeOp(44, OpType::Add, {a, b}), c});
    const std::vector<const AST::Expr*> nonAssociativeOutputs = {addRightNested, addLeftNested};
    const auto nonAssociativeFn = Codegen::EmitCFunctionMulti(nonAssociativeOutputs, 32);
    BF_TEST(nonAssociativeFn.find("uint32_t t1 = ") == std::string::npos);

    return 0;
}
