#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <ProfileEngines.h>
#include <SHA_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Testing::SHA;

namespace {

bool ContainsOp(const ExprOld* root, OpType op) {
    if (!root)
        return false;
    if (root->op == op)
        return true;
    for (const ExprOld* input : root->inputs) {
        if (ContainsOp(input, op))
            return true;
    }
    return false;
}

int TestShaSafe_RewritesCh_AndRemovesHighLevelNode() {
    Builder b;
    auto expr = b.Ch(b.Var(), b.Var(), b.Var());

    const RewriteResult out = MakeShaSafeEngine().RewriteToFixedPoint(expr);
    BF_TEST(out.StableWithoutCycle());
    BF_TEST(!ContainsOp(out.result, OpType::Ch));
    return 0;
}

int TestShaSafe_RewritesMaj_AndRemovesHighLevelNode() {
    Builder b;
    auto expr = b.Maj(b.Var(), b.Var(), b.Var());

    const RewriteResult out = MakeShaSafeEngine().RewriteToFixedPoint(expr);
    BF_TEST(out.StableWithoutCycle());
    BF_TEST(!ContainsOp(out.result, OpType::Maj));
    return 0;
}

int TestShaSafe_RewritesRoundT1Fragment_WithoutResidualChMaj() {
    Builder b;
    auto e = b.Var();
    auto f = b.Var();
    auto g = b.Var();
    auto h = b.Var();
    auto t1 = b.Add({h, b.BigSigma1(e), b.Ch(e, f, g)});

    const RewriteResult out = MakeShaSafeEngine().RewriteToFixedPoint(t1);
    BF_TEST(out.StableWithoutCycle());
    BF_TEST(!ContainsOp(out.result, OpType::Ch));
    BF_TEST(!ContainsOp(out.result, OpType::Maj));
    return 0;
}

int TestShaSafe_RewritesRoundT2Fragment_WithoutResidualChMaj() {
    Builder b;
    auto a = b.Var();
    auto bVar = b.Var();
    auto c = b.Var();
    auto d = b.Var();
    auto t2 = b.Add({d, b.BigSigma0(a), b.Maj(a, bVar, c)});

    const RewriteResult out = MakeShaSafeEngine().RewriteToFixedPoint(t2);
    BF_TEST(out.StableWithoutCycle());
    BF_TEST(!ContainsOp(out.result, OpType::Ch));
    BF_TEST(!ContainsOp(out.result, OpType::Maj));
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestShaSafe_RewritesCh_AndRemovesHighLevelNode);
    BF_RUN_TEST(TestShaSafe_RewritesMaj_AndRemovesHighLevelNode);
    BF_RUN_TEST(TestShaSafe_RewritesRoundT1Fragment_WithoutResidualChMaj);
    BF_RUN_TEST(TestShaSafe_RewritesRoundT2Fragment_WithoutResidualChMaj);
    return 0;
}
