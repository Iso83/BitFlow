#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <SHA_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Testing::SHA;
using namespace BitFlow::Core::Rules;

namespace {

using Expr = BitFlow::Core::AST::Expr;
using OpType = BitFlow::Core::AST::OpType;

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

RuleEngine MakeShaRewriteEngine() {
    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_SHA_Rules(engine);
    return engine;
}

int TestRewriteTarget_CH_ExpandsAwayHighLevelOp() {
    Builder b;
    auto x = b.Var();
    auto y = b.Var();
    auto z = b.Var();

    auto expr = b.Ch(x, y, z);
    auto rewritten = MakeShaRewriteEngine().ApplyUntilStable(expr);

    BF_TEST(!ContainsOp(rewritten, OpType::Ch));
    BF_TEST(rewritten->op == OpType::Xor);
    BF_TEST(rewritten->inputs.size() == 2);
    return 0;
}

int TestRewriteTarget_MAJ_ExpandsAwayHighLevelOp() {
    Builder b;
    auto x = b.Var();
    auto y = b.Var();
    auto z = b.Var();

    auto expr = b.Maj(x, y, z);
    auto rewritten = MakeShaRewriteEngine().ApplyUntilStable(expr);

    BF_TEST(!ContainsOp(rewritten, OpType::Maj));
    BF_TEST(rewritten->op == OpType::Xor);
    BF_TEST(rewritten->inputs.size() == 3);
    return 0;
}

int TestRewriteTarget_RoundT1Core_HasNoCHOrMAJ() {
    Builder b;
    auto e = b.Var();
    auto f = b.Var();
    auto g = b.Var();

    // Kleine round-fragment target: Sigma1(e) + Ch(e,f,g)
    auto fragment = b.Add({b.BigSigma1(e), b.Ch(e, f, g)});
    auto rewritten = MakeShaRewriteEngine().ApplyUntilStable(fragment);

    BF_TEST(rewritten->op == OpType::Add);
    BF_TEST(!ContainsOp(rewritten, OpType::Ch));
    BF_TEST(!ContainsOp(rewritten, OpType::Maj));
    return 0;
}

int TestRewriteTarget_RoundT2Core_HasNoCHOrMAJ() {
    Builder b;
    auto a = b.Var();
    auto bVar = b.Var();
    auto c = b.Var();

    // Kleine round-fragment target: Sigma0(a) + Maj(a,b,c)
    auto fragment = b.Add({b.BigSigma0(a), b.Maj(a, bVar, c)});
    auto rewritten = MakeShaRewriteEngine().ApplyUntilStable(fragment);

    BF_TEST(rewritten->op == OpType::Add);
    BF_TEST(!ContainsOp(rewritten, OpType::Ch));
    BF_TEST(!ContainsOp(rewritten, OpType::Maj));
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestRewriteTarget_CH_ExpandsAwayHighLevelOp);
    BF_RUN_TEST(TestRewriteTarget_MAJ_ExpandsAwayHighLevelOp);
    BF_RUN_TEST(TestRewriteTarget_RoundT1Core_HasNoCHOrMAJ);
    BF_RUN_TEST(TestRewriteTarget_RoundT2Core_HasNoCHOrMAJ);
    return 0;
}
