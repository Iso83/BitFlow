#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <BitFlow/io/ExprParser.h>
#include <SHA_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::Core::Testing::SHA;

namespace {

uint32_t RotR32(uint32_t x, uint32_t amount) {
    const uint32_t s = amount & 31U;
    if (s == 0U)
        return x;
    return (x >> s) | (x << (32U - s));
}

RuleEngine MakeSigmaEngine() {
    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Bitwise_Rules(engine);
    return engine;
}

int TestBigSigma0_ParseBuilderSimplifyEmitVerify() {
    constexpr uint32_t x = 305419896U; // 0x12345678

    auto parsedVar = BitFlow::IO::Parse("rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22)");
    auto rewrittenVar = MakeSigmaEngine().Rewrite(parsedVar.root);
    auto emitted = Codegen::EmitCExpr(rewrittenVar, 32);
    BF_TEST(emitted.find("bf_rotr") != std::string::npos);

    auto parsedConst = BitFlow::IO::Parse("rotr(305419896, 2) ^ rotr(305419896, 13) ^ rotr(305419896, 22)");
    const auto parsedEval = Eval::EvaluateConstant(MakeSigmaEngine().Rewrite(parsedConst.root), 32);

    Builder b;
    const auto builderEval = Eval::EvaluateConstant(MakeSigmaEngine().Rewrite(b.BigSigma0(b.Const(x))), 32);

    BF_TEST(parsedEval.status == Eval::EvalStatus::Success);
    BF_TEST(builderEval.status == Eval::EvalStatus::Success);
    BF_TEST(static_cast<uint32_t>(parsedEval.value) == (RotR32(x, 2) ^ RotR32(x, 13) ^ RotR32(x, 22)));
    BF_TEST(parsedEval.value == builderEval.value);
    return 0;
}

int TestBigSigma1_ParseBuilderSimplifyEmitVerify() {
    constexpr uint32_t x = 2309737967U; // 0x89ABCDEF

    auto parsedVar = BitFlow::IO::Parse("rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25)");
    auto rewrittenVar = MakeSigmaEngine().Rewrite(parsedVar.root);
    auto emitted = Codegen::EmitCExpr(rewrittenVar, 32);
    BF_TEST(emitted.find("bf_rotr") != std::string::npos);

    auto parsedConst = BitFlow::IO::Parse("rotr(2309737967, 6) ^ rotr(2309737967, 11) ^ rotr(2309737967, 25)");
    const auto parsedEval = Eval::EvaluateConstant(MakeSigmaEngine().Rewrite(parsedConst.root), 32);

    Builder b;
    const auto builderEval = Eval::EvaluateConstant(MakeSigmaEngine().Rewrite(b.BigSigma1(b.Const(x))), 32);

    BF_TEST(parsedEval.status == Eval::EvalStatus::Success);
    BF_TEST(builderEval.status == Eval::EvalStatus::Success);
    BF_TEST(static_cast<uint32_t>(parsedEval.value) == (RotR32(x, 6) ^ RotR32(x, 11) ^ RotR32(x, 25)));
    BF_TEST(parsedEval.value == builderEval.value);
    return 0;
}

int TestSmallSigma0_ParseBuilderSimplifyEmitVerify() {
    constexpr uint32_t x = 305419896U; // 0x12345678

    auto parsedVar = BitFlow::IO::Parse("rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3)");
    auto rewrittenVar = MakeSigmaEngine().Rewrite(parsedVar.root);
    auto emitted = Codegen::EmitCExpr(rewrittenVar, 32);
    BF_TEST(emitted.find("bf_rotr") != std::string::npos);
    BF_TEST(emitted.find(">>") != std::string::npos);

    auto parsedConst = BitFlow::IO::Parse("rotr(305419896, 7) ^ rotr(305419896, 18) ^ (305419896 >> 3)");
    const auto parsedEval = Eval::EvaluateConstant(MakeSigmaEngine().Rewrite(parsedConst.root), 32);

    Builder b;
    const auto builderEval = Eval::EvaluateConstant(MakeSigmaEngine().Rewrite(b.SmallSigma0(b.Const(x))), 32);

    BF_TEST(parsedEval.status == Eval::EvalStatus::Success);
    BF_TEST(builderEval.status == Eval::EvalStatus::Success);
    BF_TEST(static_cast<uint32_t>(parsedEval.value) == (RotR32(x, 7) ^ RotR32(x, 18) ^ (x >> 3)));
    BF_TEST(parsedEval.value == builderEval.value);
    return 0;
}

int TestSmallSigma1_ParseBuilderSimplifyEmitVerify() {
    constexpr uint32_t x = 2309737967U; // 0x89ABCDEF

    auto parsedVar = BitFlow::IO::Parse("rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10)");
    auto rewrittenVar = MakeSigmaEngine().Rewrite(parsedVar.root);
    auto emitted = Codegen::EmitCExpr(rewrittenVar, 32);
    BF_TEST(emitted.find("bf_rotr") != std::string::npos);
    BF_TEST(emitted.find(">>") != std::string::npos);

    auto parsedConst = BitFlow::IO::Parse("rotr(2309737967, 17) ^ rotr(2309737967, 19) ^ (2309737967 >> 10)");
    const auto parsedEval = Eval::EvaluateConstant(MakeSigmaEngine().Rewrite(parsedConst.root), 32);

    Builder b;
    const auto builderEval = Eval::EvaluateConstant(MakeSigmaEngine().Rewrite(b.SmallSigma1(b.Const(x))), 32);

    BF_TEST(parsedEval.status == Eval::EvalStatus::Success);
    BF_TEST(builderEval.status == Eval::EvalStatus::Success);
    BF_TEST(static_cast<uint32_t>(parsedEval.value) == (RotR32(x, 17) ^ RotR32(x, 19) ^ (x >> 10)));
    BF_TEST(parsedEval.value == builderEval.value);
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestBigSigma0_ParseBuilderSimplifyEmitVerify);
    BF_RUN_TEST(TestBigSigma1_ParseBuilderSimplifyEmitVerify);
    BF_RUN_TEST(TestSmallSigma0_ParseBuilderSimplifyEmitVerify);
    BF_RUN_TEST(TestSmallSigma1_ParseBuilderSimplifyEmitVerify);
    return 0;
}
