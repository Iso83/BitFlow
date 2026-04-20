#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <SHA_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Testing::SHA;
using namespace BitFlow::Core;
using namespace BitFlow::Core::Rules;

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

uint32_t RotR32(uint32_t x, uint32_t amount) {
    const uint32_t s = amount & 31U;
    if (s == 0U)
        return x;
    return (x >> s) | (x << (32U - s));
}

RuleEngine MakeShaNormalizeSimplifyEngine() {
    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Bitwise_Rules(engine);
    Add_Simplify_SHA_Rules(engine);
    return engine;
}

RuleEngine MakeShaFactorizeEngine() {
    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Bitwise_Rules(engine);
    Add_Factorize_Bitwise_Rules(engine);
    return engine;
}

int TestEndToEnd_T1Core_ConstantRewriteAndEmit() {
    Builder b;

    constexpr uint32_t e = 0x510E527FU;
    constexpr uint32_t f = 0x9B05688CU;
    constexpr uint32_t g = 0x1F83D9ABU;

    auto fragment = b.Add({b.BigSigma1(b.Const(e)), b.Ch(b.Const(e), b.Const(f), b.Const(g))});

    auto normalizedSimplified = MakeShaNormalizeSimplifyEngine().ApplyUntilStable(fragment);
    auto rewritten = MakeShaFactorizeEngine().ApplyRecursive(normalizedSimplified);

    BF_TEST(!ContainsOp(rewritten, OpType::Ch));
    BF_TEST(!ContainsOp(rewritten, OpType::Maj));

    const auto eval = Eval::EvaluateConstant(rewritten, 32);
    BF_TEST(eval.status == Eval::EvalStatus::Success);

    const uint32_t sigma1 = RotR32(e, 6) ^ RotR32(e, 11) ^ RotR32(e, 25);
    const uint32_t ch = (e & f) ^ ((~e) & g);
    BF_TEST(static_cast<uint32_t>(eval.value) == (sigma1 + ch));

    Builder bEmit;
    auto eVar = bEmit.Var();
    auto fVar = bEmit.Var();
    auto gVar = bEmit.Var();
    auto emitFragment = bEmit.Add({bEmit.BigSigma1(eVar), bEmit.Ch(eVar, fVar, gVar)});
    auto emitNormalizedSimplified = MakeShaNormalizeSimplifyEngine().ApplyUntilStable(emitFragment);
    auto emitRewritten = MakeShaFactorizeEngine().ApplyRecursive(emitNormalizedSimplified);
    const auto emitted = Codegen::EmitCExpr(emitRewritten, 32);
    BF_TEST(emitted.find("bf_rotr") != std::string::npos);
    BF_TEST(emitted.find("0xffffffffu") != std::string::npos);
    return 0;
}

int TestEndToEnd_T2Core_VariableRewriteAndFunctionEmit() {
    Builder b;
    auto a = b.Var();
    auto bVar = b.Var();
    auto c = b.Var();

    auto fragment = b.Add({b.BigSigma0(a), b.Maj(a, bVar, c)});
    auto normalizedSimplified = MakeShaNormalizeSimplifyEngine().ApplyUntilStable(fragment);
    auto rewritten = MakeShaFactorizeEngine().ApplyRecursive(normalizedSimplified);

    BF_TEST(!ContainsOp(rewritten, OpType::Ch));
    BF_TEST(!ContainsOp(rewritten, OpType::Maj));

    const auto fn = Codegen::EmitCFunction(rewritten, 32);
    BF_TEST(fn.find("uint32_t eval(") != std::string::npos);
    BF_TEST(fn.find("bf_rotr") != std::string::npos);
    BF_TEST(fn.find("return eval(") != std::string::npos);
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestEndToEnd_T1Core_ConstantRewriteAndEmit);
    BF_RUN_TEST(TestEndToEnd_T2Core_VariableRewriteAndFunctionEmit);
    return 0;
}
