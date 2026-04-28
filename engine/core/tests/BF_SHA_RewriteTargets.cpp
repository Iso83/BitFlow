#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <ProfileEngines.h>
#include <SHA_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Testing::SHA;
using namespace BitFlow::Core::Rules;
using namespace BitFlow::Core::Expression;

namespace {

bool ContainsOp(const ExprOld* root, OpType op) {
    if (root == nullptr)
        return false;

    if (root->op == op)
        return true;

    for (const ExprOld* in : root->inputs) {
        if (ContainsOp(in, op))
            return true;
    }

    return false;
}

bool IsAndPair(const ExprOld* e, const ExprOld* a, const ExprOld* b) {
    if (!e || e->op != OpType::And || e->inputs.size() != 2)
        return false;

    const ExprOld* lhs = e->inputs[0];
    const ExprOld* rhs = e->inputs[1];
    return (lhs->id.value() == a->id.value() && rhs->id.value() == b->id.value()) ||
           (lhs->id.value() == b->id.value() && rhs->id.value() == a->id.value());
}

bool IsAndNotPair(const ExprOld* e, const ExprOld* x, const ExprOld* z) {
    if (!e || e->op != OpType::And || e->inputs.size() != 2)
        return false;

    const ExprOld* lhs = e->inputs[0];
    const ExprOld* rhs = e->inputs[1];

    auto isNotOfX = [&](const ExprOld* candidate) {
        return candidate && candidate->op == OpType::Not && candidate->inputs.size() == 1 &&
               candidate->inputs[0]->id.value() == x->id.value();
    };

    return (isNotOfX(lhs) && rhs->id.value() == z->id.value()) || (isNotOfX(rhs) && lhs->id.value() == z->id.value());
}

bool IsCanonicalCH(const ExprOld* e, const ExprOld* x, const ExprOld* y, const ExprOld* z) {
    if (!e || e->op != OpType::Xor || e->inputs.size() != 2)
        return false;

    return (IsAndPair(e->inputs[0], x, y) && IsAndNotPair(e->inputs[1], x, z)) ||
           (IsAndPair(e->inputs[1], x, y) && IsAndNotPair(e->inputs[0], x, z));
}

bool IsCanonicalMAJ(const ExprOld* e, const ExprOld* x, const ExprOld* y, const ExprOld* z) {
    if (!e || e->op != OpType::Xor || e->inputs.size() != 3)
        return false;

    bool hasXY = false;
    bool hasXZ = false;
    bool hasYZ = false;
    for (const ExprOld* in : e->inputs) {
        hasXY = hasXY || IsAndPair(in, x, y);
        hasXZ = hasXZ || IsAndPair(in, x, z);
        hasYZ = hasYZ || IsAndPair(in, y, z);
    }

    return hasXY && hasXZ && hasYZ;
}

bool ContainsCanonicalCH(const ExprOld* root, const ExprOld* x, const ExprOld* y, const ExprOld* z) {
    if (root == nullptr)
        return false;

    if (IsCanonicalCH(root, x, y, z))
        return true;

    for (const ExprOld* in : root->inputs) {
        if (ContainsCanonicalCH(in, x, y, z))
            return true;
    }

    return false;
}

bool ContainsCanonicalMAJ(const ExprOld* root, const ExprOld* x, const ExprOld* y, const ExprOld* z) {
    if (root == nullptr)
        return false;

    if (IsCanonicalMAJ(root, x, y, z))
        return true;

    for (const ExprOld* in : root->inputs) {
        if (ContainsCanonicalMAJ(in, x, y, z))
            return true;
    }

    return false;
}

int TestRewriteTarget_CH_ExpandsAwayHighLevelOp() {
    Builder b;
    auto x = b.Var();
    auto y = b.Var();
    auto z = b.Var();

    auto expr = b.Ch(x, y, z);
    auto rewritten = MakeShaSafeEngine().Rewrite(expr);

    BF_TEST(!ContainsOp(rewritten, OpType::Ch));
    BF_TEST(IsCanonicalCH(rewritten, x, y, z));
    return 0;
}

int TestRewriteTarget_MAJ_ExpandsAwayHighLevelOp() {
    Builder b;
    auto x = b.Var();
    auto y = b.Var();
    auto z = b.Var();

    auto expr = b.Maj(x, y, z);
    auto rewritten = MakeShaSafeEngine().Rewrite(expr);

    BF_TEST(!ContainsOp(rewritten, OpType::Maj));
    BF_TEST(IsCanonicalMAJ(rewritten, x, y, z));
    return 0;
}

int TestRewriteTarget_CH_EquivalentForm_ConvergesToCanonical() {
    Builder b;
    auto x = b.Var();
    auto y = b.Var();
    auto z = b.Var();

    // Equivalent CH vorm: z ^ (x & (y ^ z))
    auto expr = b.Xor({z, b.And(x, b.Xor({y, z}))});
    auto rewritten = MakeShaSafeEngine().Rewrite(expr);

    BF_TEST(IsCanonicalCH(rewritten, x, y, z));
    return 0;
}

int TestRewriteTarget_MAJ_EquivalentOrForm_ConvergesToCanonical() {
    Builder b;
    auto x = b.Var();
    auto y = b.Var();
    auto z = b.Var();

    // Equivalent MAJ vorm: (x & y) | (z & (x ^ y))
    auto expr = MakeOp(9999, OpType::Or, {b.And(x, y), b.And(z, b.Xor({x, y}))});
    auto rewritten = MakeShaSafeEngine().Rewrite(expr);

    BF_TEST(IsCanonicalMAJ(rewritten, x, y, z));
    return 0;
}

int TestRewriteTarget_RoundT1Core_HasNoCHOrMAJ() {
    Builder b;
    auto e = b.Var();
    auto f = b.Var();
    auto g = b.Var();

    // Kleine round-fragment target: Sigma1(e) + Ch(e,f,g)
    auto fragment = b.Add({b.BigSigma1(e), b.Ch(e, f, g)});
    auto rewritten = MakeShaSafeEngine().Rewrite(fragment);

    BF_TEST(rewritten->op == OpType::Add);
    BF_TEST(!ContainsOp(rewritten, OpType::Ch));
    BF_TEST(!ContainsOp(rewritten, OpType::Maj));
    BF_TEST(ContainsCanonicalCH(rewritten, e, f, g));
    return 0;
}

int TestRewriteTarget_RoundT2Core_HasNoCHOrMAJ() {
    Builder b;
    auto a = b.Var();
    auto bVar = b.Var();
    auto c = b.Var();

    // Kleine round-fragment target: Sigma0(a) + Maj(a,b,c)
    auto fragment = b.Add({b.BigSigma0(a), b.Maj(a, bVar, c)});
    auto rewritten = MakeShaSafeEngine().Rewrite(fragment);

    BF_TEST(rewritten->op == OpType::Add);
    BF_TEST(!ContainsOp(rewritten, OpType::Ch));
    BF_TEST(!ContainsOp(rewritten, OpType::Maj));
    BF_TEST(ContainsCanonicalMAJ(rewritten, a, bVar, c));
    return 0;
}

int TestRewriteTarget_RoundFragment_CH_InLargerExpr_UsesCanonicalSubform() {
    Builder b;
    auto e = b.Var();
    auto f = b.Var();
    auto g = b.Var();
    auto h = b.Var();

    // Mini-fragment: een grotere expressie waarin CH als subexpressie zit.
    auto fragment = b.Add({h, b.BigSigma1(e), b.Ch(e, f, g)});
    auto rewritten = MakeShaSafeEngine().Rewrite(fragment);

    BF_TEST(rewritten->op == OpType::Add);
    BF_TEST(!ContainsOp(rewritten, OpType::Ch));
    BF_TEST(ContainsCanonicalCH(rewritten, e, f, g));
    return 0;
}

int TestRewriteTarget_RoundFragment_MAJ_InLargerExpr_UsesCanonicalSubform() {
    Builder b;
    auto a = b.Var();
    auto bVar = b.Var();
    auto c = b.Var();
    auto d = b.Var();

    // Mini-fragment: een grotere expressie waarin MAJ als subexpressie zit.
    auto fragment = b.Add({d, b.BigSigma0(a), b.Maj(a, bVar, c)});
    auto rewritten = MakeShaSafeEngine().Rewrite(fragment);

    BF_TEST(rewritten->op == OpType::Add);
    BF_TEST(!ContainsOp(rewritten, OpType::Maj));
    BF_TEST(ContainsCanonicalMAJ(rewritten, a, bVar, c));
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestRewriteTarget_CH_ExpandsAwayHighLevelOp);
    BF_RUN_TEST(TestRewriteTarget_MAJ_ExpandsAwayHighLevelOp);
    BF_RUN_TEST(TestRewriteTarget_CH_EquivalentForm_ConvergesToCanonical);
    BF_RUN_TEST(TestRewriteTarget_MAJ_EquivalentOrForm_ConvergesToCanonical);
    BF_RUN_TEST(TestRewriteTarget_RoundT1Core_HasNoCHOrMAJ);
    BF_RUN_TEST(TestRewriteTarget_RoundT2Core_HasNoCHOrMAJ);
    BF_RUN_TEST(TestRewriteTarget_RoundFragment_CH_InLargerExpr_UsesCanonicalSubform);
    BF_RUN_TEST(TestRewriteTarget_RoundFragment_MAJ_InLargerExpr_UsesCanonicalSubform);
    return 0;
}
