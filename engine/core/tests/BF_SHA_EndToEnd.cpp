#include <BitFlow/core/codegen/Emitter.h>
#include <BitFlow/core/eval/ConstantEval.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <ProfileEngines.h>
#include <SHA_Expr.h>
#include <TestAssert.h>
#include <array>
#include <cstdint>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Testing::SHA;
using namespace BitFlow::Core;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::Core::Expression;

namespace {

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

RuleEngine MakeShaFactorizeEngine() {
    return BuildProfile("factorize_bitwise_safe");
}

Expr* RunRoundFragmentPipeline(Expr* fragment) {
    auto normalizedSimplified = MakeShaSafeEngine().Rewrite(fragment);
    return MakeShaFactorizeEngine().ApplyRecursive(normalizedSimplified);
}

int TestEndToEnd_T1SigmaChoiceCore_ManyConcreteCases() {
    constexpr std::array<std::array<uint32_t, 3>, 4> cases = {{{0x510E527FU, 0x9B05688CU, 0x1F83D9ABU},
                                                               {0x00000000U, 0xFFFFFFFFU, 0xA5A5A5A5U},
                                                               {0x13579BDFU, 0x2468ACE0U, 0x0F0FF0F0U},
                                                               {0xCAFEBABEU, 0xDEADBEEFU, 0x10203040U}}};

    for (const auto& tc : cases) {
        const uint32_t e = tc[0];
        const uint32_t f = tc[1];
        const uint32_t g = tc[2];
        Builder bNs;
        auto nsExpr = bNs.RoundT1SigmaChoiceCore(bNs.Const(e), bNs.Const(f), bNs.Const(g));
        auto normalizedSimplified = MakeShaSafeEngine().Rewrite(nsExpr);

        Builder bRw;
        auto rewritten = RunRoundFragmentPipeline(bRw.RoundT1SigmaChoiceCore(bRw.Const(e), bRw.Const(f), bRw.Const(g)));

        BF_TEST(!ContainsOp(normalizedSimplified, OpType::Ch));
        BF_TEST(!ContainsOp(normalizedSimplified, OpType::Maj));
        BF_TEST(!ContainsOp(rewritten, OpType::Ch));
        BF_TEST(!ContainsOp(rewritten, OpType::Maj));

        const auto evalSimplified = Eval::EvaluateConstant(normalizedSimplified, 32);
        const auto evalRewritten = Eval::EvaluateConstant(rewritten, 32);
        BF_TEST(evalSimplified.status == Eval::EvalStatus::Success);
        BF_TEST(evalRewritten.status == Eval::EvalStatus::Success);
        BF_TEST(evalRewritten.value == evalSimplified.value);
    }

    Builder bEmit;
    auto eVar = bEmit.Var();
    auto fVar = bEmit.Var();
    auto gVar = bEmit.Var();
    auto emitFragment = bEmit.RoundT1SigmaChoiceCore(eVar, fVar, gVar);
    const auto emittedBefore = Codegen::EmitCExpr(emitFragment, 32);
    const auto emittedAfter = Codegen::EmitCExpr(RunRoundFragmentPipeline(emitFragment), 32);

    const auto emittedAfterAgain = Codegen::EmitCExpr(RunRoundFragmentPipeline(emitFragment), 32);

    BF_TEST(emittedBefore != emittedAfter);
    BF_TEST(emittedAfter == emittedAfterAgain);
    BF_TEST(emittedAfter.find("bf_rotr") != std::string::npos);
    BF_TEST(emittedAfter.find("0xffffffffu") != std::string::npos);
    return 0;
}

int TestEndToEnd_T2PartCore_ManyConcreteCases() {
    constexpr std::array<std::array<uint32_t, 3>, 4> cases = {{{0x6A09E667U, 0xBB67AE85U, 0x3C6EF372U},
                                                               {0x00000000U, 0xFFFFFFFFU, 0x12345678U},
                                                               {0x13579BDFU, 0x2468ACE0U, 0x0F0FF0F0U},
                                                               {0xCAFEBABEU, 0xDEADBEEFU, 0x10203040U}}};

    for (const auto& tc : cases) {
        const uint32_t a = tc[0];
        const uint32_t b = tc[1];
        const uint32_t c = tc[2];
        Builder bNs;
        auto nsExpr = bNs.RoundT2PartCore(bNs.Const(a), bNs.Const(b), bNs.Const(c));
        auto normalizedSimplified = MakeShaSafeEngine().Rewrite(nsExpr);

        Builder bRw;
        auto rewritten = RunRoundFragmentPipeline(bRw.RoundT2PartCore(bRw.Const(a), bRw.Const(b), bRw.Const(c)));

        BF_TEST(!ContainsOp(normalizedSimplified, OpType::Ch));
        BF_TEST(!ContainsOp(normalizedSimplified, OpType::Maj));
        BF_TEST(!ContainsOp(rewritten, OpType::Ch));
        BF_TEST(!ContainsOp(rewritten, OpType::Maj));

        const auto evalSimplified = Eval::EvaluateConstant(normalizedSimplified, 32);
        const auto evalRewritten = Eval::EvaluateConstant(rewritten, 32);
        BF_TEST(evalSimplified.status == Eval::EvalStatus::Success);
        BF_TEST(evalRewritten.status == Eval::EvalStatus::Success);
        BF_TEST(evalRewritten.value == evalSimplified.value);
    }

    Builder bEmit;
    auto aVar = bEmit.Var();
    auto bVar = bEmit.Var();
    auto cVar = bEmit.Var();
    auto emitFragment = bEmit.RoundT2PartCore(aVar, bVar, cVar);
    const auto emittedBefore = Codegen::EmitCExpr(emitFragment, 32);
    const auto emittedAfter = Codegen::EmitCExpr(RunRoundFragmentPipeline(emitFragment), 32);

    const auto emittedAfterAgain = Codegen::EmitCExpr(RunRoundFragmentPipeline(emitFragment), 32);

    BF_TEST(emittedBefore != emittedAfter);
    BF_TEST(emittedAfter == emittedAfterAgain);
    BF_TEST(emittedAfter.find("bf_rotr") != std::string::npos);
    return 0;
}

int TestEndToEnd_T1PartCore_ManyConcreteCases() {
    constexpr std::array<std::array<uint32_t, 4>, 4> cases = {{{0x5BE0CD19U, 0x510E527FU, 0x9B05688CU, 0x1F83D9ABU},
                                                               {0x00000000U, 0xFFFFFFFFU, 0xAAAAAAAAU, 0x55555555U},
                                                               {0x13579BDFU, 0x2468ACE0U, 0x0F0FF0F0U, 0xF0F00F0FU},
                                                               {0xCAFEBABEU, 0xDEADBEEFU, 0x10203040U, 0x55667788U}}};

    for (const auto& tc : cases) {
        const uint32_t h = tc[0];
        const uint32_t e = tc[1];
        const uint32_t f = tc[2];
        const uint32_t g = tc[3];
        Builder bNs;
        auto nsExpr = bNs.RoundT1PartCore(bNs.Const(h), bNs.Const(e), bNs.Const(f), bNs.Const(g));
        auto normalizedSimplified = MakeShaSafeEngine().Rewrite(nsExpr);

        Builder bRw;
        auto rewritten =
            RunRoundFragmentPipeline(bRw.RoundT1PartCore(bRw.Const(h), bRw.Const(e), bRw.Const(f), bRw.Const(g)));

        BF_TEST(!ContainsOp(normalizedSimplified, OpType::Ch));
        BF_TEST(!ContainsOp(normalizedSimplified, OpType::Maj));
        BF_TEST(!ContainsOp(rewritten, OpType::Ch));
        BF_TEST(!ContainsOp(rewritten, OpType::Maj));

        const auto evalSimplified = Eval::EvaluateConstant(normalizedSimplified, 32);
        const auto evalRewritten = Eval::EvaluateConstant(rewritten, 32);
        BF_TEST(evalSimplified.status == Eval::EvalStatus::Success);
        BF_TEST(evalRewritten.status == Eval::EvalStatus::Success);
        BF_TEST(evalRewritten.value == evalSimplified.value);
    }

    Builder bEmit;
    auto hVar = bEmit.Var();
    auto eVar = bEmit.Var();
    auto fVar = bEmit.Var();
    auto gVar = bEmit.Var();
    auto emitFragment = bEmit.RoundT1PartCore(hVar, eVar, fVar, gVar);
    const auto emittedBefore = Codegen::EmitCExpr(emitFragment, 32);
    const auto emittedAfter = Codegen::EmitCExpr(RunRoundFragmentPipeline(emitFragment), 32);

    const auto emittedAfterAgain = Codegen::EmitCExpr(RunRoundFragmentPipeline(emitFragment), 32);

    BF_TEST(emittedBefore != emittedAfter);
    BF_TEST(emittedAfter == emittedAfterAgain);
    BF_TEST(emittedAfter.find("bf_rotr") != std::string::npos);

    Builder bCompact;
    auto compactExpr = bCompact.RoundT1PartCore(bCompact.Const(0U), bCompact.Var(), bCompact.Var(), bCompact.Var());
    const auto compactBefore = Codegen::EmitCExpr(compactExpr, 32);
    const auto compactAfter = Codegen::EmitCExpr(RunRoundFragmentPipeline(compactExpr), 32);
    BF_TEST(compactAfter.find("0x00000000u") == std::string::npos);
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestEndToEnd_T1SigmaChoiceCore_ManyConcreteCases);
    BF_RUN_TEST(TestEndToEnd_T2PartCore_ManyConcreteCases);
    BF_RUN_TEST(TestEndToEnd_T1PartCore_ManyConcreteCases);
    return 0;
}
