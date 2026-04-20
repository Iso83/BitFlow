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

Expr* RunRoundFragmentPipeline(Expr* fragment) {
    auto normalizedSimplified = MakeShaNormalizeSimplifyEngine().ApplyUntilStable(fragment);
    return MakeShaFactorizeEngine().ApplyRecursive(normalizedSimplified);
}

int TestEndToEnd_T1SigmaChoiceCore_AnalyzeTargetFlow() {
    Builder b;

    constexpr uint32_t e = 0x510E527FU;
    constexpr uint32_t f = 0x9B05688CU;
    constexpr uint32_t g = 0x1F83D9ABU;

    auto fragment = b.RoundT1SigmaChoiceCore(b.Const(e), b.Const(f), b.Const(g));
    auto rewritten = RunRoundFragmentPipeline(fragment);

    BF_TEST(!ContainsOp(rewritten, OpType::Ch));
    BF_TEST(!ContainsOp(rewritten, OpType::Maj));

    auto normalizedSimplified = MakeShaNormalizeSimplifyEngine().ApplyUntilStable(fragment);

    const auto evalSimplified = Eval::EvaluateConstant(normalizedSimplified, 32);
    const auto evalRewritten = Eval::EvaluateConstant(rewritten, 32);
    BF_TEST(evalSimplified.status == Eval::EvalStatus::Success);
    BF_TEST(evalRewritten.status == Eval::EvalStatus::Success);

    BF_TEST(evalRewritten.value == evalSimplified.value);

    Builder bEmit;
    auto eVar = bEmit.Var();
    auto fVar = bEmit.Var();
    auto gVar = bEmit.Var();
    auto emitFragment = bEmit.RoundT1SigmaChoiceCore(eVar, fVar, gVar);
    const auto emittedBefore = Codegen::EmitCExpr(emitFragment, 32);
    const auto emittedAfter = Codegen::EmitCExpr(RunRoundFragmentPipeline(emitFragment), 32);

    BF_TEST(emittedBefore != emittedAfter);
    BF_TEST(emittedAfter.find("bf_rotr") != std::string::npos);
    BF_TEST(emittedAfter.find("0xffffffffu") != std::string::npos);
    return 0;
}

int TestEndToEnd_T2PartCore_AnalyzeTargetFlow() {
    Builder b;

    constexpr uint32_t a = 0x6A09E667U;
    constexpr uint32_t bVal = 0xBB67AE85U;
    constexpr uint32_t c = 0x3C6EF372U;

    auto fragment = b.RoundT2PartCore(b.Const(a), b.Const(bVal), b.Const(c));
    auto rewritten = RunRoundFragmentPipeline(fragment);

    BF_TEST(!ContainsOp(rewritten, OpType::Ch));
    BF_TEST(!ContainsOp(rewritten, OpType::Maj));

    auto normalizedSimplified = MakeShaNormalizeSimplifyEngine().ApplyUntilStable(fragment);

    const auto evalSimplified = Eval::EvaluateConstant(normalizedSimplified, 32);
    const auto evalRewritten = Eval::EvaluateConstant(rewritten, 32);
    BF_TEST(evalSimplified.status == Eval::EvalStatus::Success);
    BF_TEST(evalRewritten.status == Eval::EvalStatus::Success);

    BF_TEST(evalRewritten.value == evalSimplified.value);

    Builder bEmit;
    auto aVar = bEmit.Var();
    auto bVar = bEmit.Var();
    auto cVar = bEmit.Var();
    auto emitFragment = bEmit.RoundT2PartCore(aVar, bVar, cVar);
    const auto emittedBefore = Codegen::EmitCExpr(emitFragment, 32);
    const auto emittedAfter = Codegen::EmitCExpr(RunRoundFragmentPipeline(emitFragment), 32);

    BF_TEST(emittedBefore != emittedAfter);
    BF_TEST(emittedAfter.find("bf_rotr") != std::string::npos);
    BF_TEST(emittedAfter.find("return eval(") == std::string::npos);
    return 0;
}

int TestEndToEnd_T1ChoiceOnly_AnalyzeTargetFlow() {
    Builder b;

    constexpr uint32_t e = 0x510E527FU;
    constexpr uint32_t f = 0x9B05688CU;
    constexpr uint32_t g = 0x1F83D9ABU;

    auto fragment = b.RoundT1ChoiceCore(b.Const(e), b.Const(f), b.Const(g));
    auto rewritten = RunRoundFragmentPipeline(fragment);

    BF_TEST(!ContainsOp(rewritten, OpType::Ch));
    BF_TEST(!ContainsOp(rewritten, OpType::Maj));

    auto normalizedSimplified = MakeShaNormalizeSimplifyEngine().ApplyUntilStable(fragment);

    const auto evalSimplified = Eval::EvaluateConstant(normalizedSimplified, 32);
    const auto evalRewritten = Eval::EvaluateConstant(rewritten, 32);
    BF_TEST(evalSimplified.status == Eval::EvalStatus::Success);
    BF_TEST(evalRewritten.status == Eval::EvalStatus::Success);

    BF_TEST(evalRewritten.value == evalSimplified.value);

    Builder bEmit;
    auto eVar = bEmit.Var();
    auto fVar = bEmit.Var();
    auto gVar = bEmit.Var();
    auto emitFragment = bEmit.RoundT1ChoiceCore(eVar, fVar, gVar);
    const auto emittedBefore = Codegen::EmitCExpr(emitFragment, 32);
    const auto emittedAfter = Codegen::EmitCExpr(RunRoundFragmentPipeline(emitFragment), 32);

    BF_TEST(emittedBefore != emittedAfter);
    BF_TEST(emittedAfter.find("&") != std::string::npos);
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestEndToEnd_T1SigmaChoiceCore_AnalyzeTargetFlow);
    BF_RUN_TEST(TestEndToEnd_T2PartCore_AnalyzeTargetFlow);
    BF_RUN_TEST(TestEndToEnd_T1ChoiceOnly_AnalyzeTargetFlow);
    return 0;
}
