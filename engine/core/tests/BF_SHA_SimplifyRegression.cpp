#include <BitFlow/core/eval/ConstantEval.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <ProfileEngines.h>
#include <SHA_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Testing::SHA;

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

int TestSimplify_CH_EliminatedAndEquivalent() {
    Builder b;
    auto expr = b.Ch(b.Var(), b.Const(50U), b.Const(30U));
    auto rewritten = MakeShaSafeEngine().ApplyUntilStable(expr);

    BF_TEST(!ContainsOp(rewritten, OpType::Ch));

    const auto eval = Eval::EvaluateConstant(rewritten, 32);
    BF_TEST(eval.status == Eval::EvalStatus::NotConstant);
    return 0;
}

int TestSimplify_MAJ_EliminatedAndEquivalent() {
    Builder b;
    auto expr = b.Maj(b.Var(), b.Const(50U), b.Const(30U));
    auto rewritten = MakeShaSafeEngine().ApplyUntilStable(expr);

    BF_TEST(!ContainsOp(rewritten, OpType::Maj));

    const auto eval = Eval::EvaluateConstant(rewritten, 32);
    BF_TEST(eval.status == Eval::EvalStatus::NotConstant);
    return 0;
}

int TestSimplify_CH_ConstantSemantics() {
    Builder b;
    constexpr uint32_t x = 0xA5A5A5A5U;
    constexpr uint32_t y = 50U;
    constexpr uint32_t z = 30U;

    auto expr = b.Ch(b.Const(x), b.Const(y), b.Const(z));
    auto rewritten = MakeShaSafeEngine().ApplyUntilStable(expr);

    const auto eval = Eval::EvaluateConstant(rewritten, 32);
    BF_TEST(eval.status == Eval::EvalStatus::Success);
    BF_TEST(static_cast<uint32_t>(eval.value) == ((x & y) ^ ((~x) & z)));
    return 0;
}

int TestSimplify_MAJ_ConstantSemantics() {
    Builder b;
    constexpr uint32_t x = 0xA5A5A5A5U;
    constexpr uint32_t y = 50U;
    constexpr uint32_t z = 30U;

    auto expr = b.Maj(b.Const(x), b.Const(y), b.Const(z));
    auto rewritten = MakeShaSafeEngine().ApplyUntilStable(expr);

    const auto eval = Eval::EvaluateConstant(rewritten, 32);
    BF_TEST(eval.status == Eval::EvalStatus::Success);
    BF_TEST(static_cast<uint32_t>(eval.value) == ((x & y) ^ (x & z) ^ (y & z)));
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestSimplify_CH_EliminatedAndEquivalent);
    BF_RUN_TEST(TestSimplify_MAJ_EliminatedAndEquivalent);
    BF_RUN_TEST(TestSimplify_CH_ConstantSemantics);
    BF_RUN_TEST(TestSimplify_MAJ_ConstantSemantics);
    return 0;
}
