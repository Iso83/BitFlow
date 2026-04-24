#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <ProfileEngines.h>
#include <SHA_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Testing::SHA;

namespace {

using OpType = AST::OpType;

uint32_t RotR32(uint32_t x, uint32_t amount) {
    const uint32_t s = amount & 31U;
    if (s == 0U)
        return x;
    return (x >> s) | (x << (32U - s));
}

int TestSmallSigma0_BuilderRecipe() {
    Builder b;
    auto x = b.Var();
    auto sigma = b.SmallSigma0(x);

    BF_TEST(sigma->op == OpType::Xor);
    BF_TEST(sigma->inputs.size() == 3);
    BF_TEST(sigma->inputs[0]->op == OpType::RotR);
    BF_TEST(sigma->inputs[0]->inputs[1]->constValue == 7U);
    BF_TEST(sigma->inputs[1]->op == OpType::RotR);
    BF_TEST(sigma->inputs[1]->inputs[1]->constValue == 18U);
    BF_TEST(sigma->inputs[2]->op == OpType::Shr);
    BF_TEST(sigma->inputs[2]->inputs[1]->constValue == 3U);
    return 0;
}

int TestSmallSigma1_BuilderRecipe() {
    Builder b;
    auto x = b.Var();
    auto sigma = b.SmallSigma1(x);

    BF_TEST(sigma->op == OpType::Xor);
    BF_TEST(sigma->inputs.size() == 3);
    BF_TEST(sigma->inputs[0]->op == OpType::RotR);
    BF_TEST(sigma->inputs[0]->inputs[1]->constValue == 17U);
    BF_TEST(sigma->inputs[1]->op == OpType::RotR);
    BF_TEST(sigma->inputs[1]->inputs[1]->constValue == 19U);
    BF_TEST(sigma->inputs[2]->op == OpType::Shr);
    BF_TEST(sigma->inputs[2]->inputs[1]->constValue == 10U);
    return 0;
}

int TestSmallSigma0_RewriteEmitEvalConsistency() {
    Builder b;
    constexpr uint32_t x = 0x12345678U;

    auto expr = b.SmallSigma0(b.Const(x));
    auto rewritten = MakeShaSafeEngine().ApplyUntilStable(expr);

    const auto eval = Eval::EvaluateConstant(rewritten, 32);
    BF_TEST(eval.status == Eval::EvalStatus::Success);
    BF_TEST(static_cast<uint32_t>(eval.value) == (RotR32(x, 7) ^ RotR32(x, 18) ^ (x >> 3)));

    auto sigmaVar = b.SmallSigma0(b.Var());
    auto emitted = Codegen::EmitCExpr(MakeShaSafeEngine().ApplyUntilStable(sigmaVar), 32);
    BF_TEST(emitted.find("bf_rotr") != std::string::npos);
    BF_TEST(emitted.find(">>") != std::string::npos);
    return 0;
}

int TestSmallSigma1_RewriteEmitEvalConsistency() {
    Builder b;
    constexpr uint32_t x = 0x89ABCDEFU;

    auto expr = b.SmallSigma1(b.Const(x));
    auto rewritten = MakeShaSafeEngine().ApplyUntilStable(expr);

    const auto eval = Eval::EvaluateConstant(rewritten, 32);
    BF_TEST(eval.status == Eval::EvalStatus::Success);
    BF_TEST(static_cast<uint32_t>(eval.value) == (RotR32(x, 17) ^ RotR32(x, 19) ^ (x >> 10)));

    auto sigmaVar = b.SmallSigma1(b.Var());
    auto emitted = Codegen::EmitCExpr(MakeShaSafeEngine().ApplyUntilStable(sigmaVar), 32);
    BF_TEST(emitted.find("bf_rotr") != std::string::npos);
    BF_TEST(emitted.find(">>") != std::string::npos);
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestSmallSigma0_BuilderRecipe);
    BF_RUN_TEST(TestSmallSigma1_BuilderRecipe);
    BF_RUN_TEST(TestSmallSigma0_RewriteEmitEvalConsistency);
    BF_RUN_TEST(TestSmallSigma1_RewriteEmitEvalConsistency);
    return 0;
}
