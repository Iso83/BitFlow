#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <SHA_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Testing::SHA;

namespace {

using Expr = AST::Expr;
using OpType = AST::OpType;

bool ContainsOp(const Expr* root, OpType op) {
    if (root == nullptr)
        return false;
    if (root->op == op)
        return true;
    for (const Expr* in : root->inputs) {
        if (ContainsOp(in, op))
            return true;
    }
    return false;
}

RuleEngine MakeShaNormalizeSimplifyEngine() {
    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Bitwise_Rules(engine);
    Add_Simplify_SHA_Rules(engine);
    return engine;
}

uint32_t RotR32(uint32_t x, uint32_t amount) {
    const uint32_t s = amount & 31U;
    if (s == 0U)
        return x;
    return (x >> s) | (x << (32U - s));
}

int TestFragment_CH_RewriteAndConstantEval() {
    Builder b;
    constexpr uint32_t x = 0x12345678U;
    constexpr uint32_t y = 0xF0F0F0F0U;
    constexpr uint32_t z = 0x00FF00FFU;

    auto ch = b.Ch(b.Const(x), b.Const(y), b.Const(z));
    auto rewritten = MakeShaNormalizeSimplifyEngine().ApplyUntilStable(ch);

    BF_TEST(!ContainsOp(rewritten, OpType::Ch));

    const auto eval = Eval::EvaluateConstant(rewritten, 32);
    BF_TEST(eval.status == Eval::EvalStatus::Success);
    BF_TEST(static_cast<uint32_t>(eval.value) == ((x & y) ^ ((~x) & z)));
    return 0;
}

int TestFragment_MAJ_RewriteAndConstantEval() {
    Builder b;
    constexpr uint32_t x = 0x01234567U;
    constexpr uint32_t y = 0x89ABCDEFU;
    constexpr uint32_t z = 0x0F0FF0F0U;

    auto maj = b.Maj(b.Const(x), b.Const(y), b.Const(z));
    auto rewritten = MakeShaNormalizeSimplifyEngine().ApplyUntilStable(maj);

    BF_TEST(!ContainsOp(rewritten, OpType::Maj));

    const auto eval = Eval::EvaluateConstant(rewritten, 32);
    BF_TEST(eval.status == Eval::EvalStatus::Success);
    BF_TEST(static_cast<uint32_t>(eval.value) == ((x & y) ^ (x & z) ^ (y & z)));
    return 0;
}

int TestFragment_Sigma1_EmitUsesRotateRuntimeContract() {
    Builder b;
    constexpr uint32_t e = 0x510E527FU;

    auto sigma1 = b.BigSigma1(b.Const(e));
    const auto eval = Eval::EvaluateConstant(sigma1, 32);
    BF_TEST(eval.status == Eval::EvalStatus::Success);

    const uint32_t expected = RotR32(e, 6) ^ RotR32(e, 11) ^ RotR32(e, 25);
    BF_TEST(static_cast<uint32_t>(eval.value) == expected);

    auto v = b.Var();
    auto sigma1Var = b.BigSigma1(v);
    const auto emitted = Codegen::EmitCExpr(sigma1Var, 32);
    BF_TEST(emitted.find("bf_rotr") != std::string::npos);
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestFragment_CH_RewriteAndConstantEval);
    BF_RUN_TEST(TestFragment_MAJ_RewriteAndConstantEval);
    BF_RUN_TEST(TestFragment_Sigma1_EmitUsesRotateRuntimeContract);
    return 0;
}
