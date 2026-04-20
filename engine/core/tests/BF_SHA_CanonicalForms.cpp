#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <SHA_Expr.h>
#include <TestAssert.h>
#include <string>

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

RuleEngine MakeSigmaNormalizeEngine() {
    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Bitwise_Rules(engine);
    return engine;
}

std::string ExprSignature(const Expr* root) {
    if (root == nullptr)
        return "null";

    std::string sig = std::to_string(static_cast<int>(root->op));
    sig += ":";
    sig += std::to_string(root->constValue);
    sig += "[";
    for (size_t i = 0; i < root->inputs.size(); ++i) {
        if (i != 0)
            sig += ",";
        sig += ExprSignature(root->inputs[i]);
    }
    sig += "]";
    return sig;
}

bool IsAndPair(const Expr* e, const Expr* a, const Expr* b) {
    if (!e || e->op != OpType::And || e->inputs.size() != 2)
        return false;

    const Expr* lhs = e->inputs[0];
    const Expr* rhs = e->inputs[1];
    return (lhs->id.value() == a->id.value() && rhs->id.value() == b->id.value()) ||
           (lhs->id.value() == b->id.value() && rhs->id.value() == a->id.value());
}

bool IsAndNotPair(const Expr* e, const Expr* x, const Expr* z) {
    if (!e || e->op != OpType::And || e->inputs.size() != 2)
        return false;

    const Expr* lhs = e->inputs[0];
    const Expr* rhs = e->inputs[1];

    auto isNotOfX = [&](const Expr* candidate) {
        return candidate && candidate->op == OpType::Not && candidate->inputs.size() == 1 &&
               candidate->inputs[0]->id.value() == x->id.value();
    };

    return (isNotOfX(lhs) && rhs->id.value() == z->id.value()) || (isNotOfX(rhs) && lhs->id.value() == z->id.value());
}

bool IsCanonicalCH(const Expr* e, const Expr* x, const Expr* y, const Expr* z) {
    if (!e || e->op != OpType::Xor || e->inputs.size() != 2)
        return false;

    return (IsAndPair(e->inputs[0], x, y) && IsAndNotPair(e->inputs[1], x, z)) ||
           (IsAndPair(e->inputs[1], x, y) && IsAndNotPair(e->inputs[0], x, z));
}

bool IsCanonicalMAJ(const Expr* e, const Expr* x, const Expr* y, const Expr* z) {
    if (!e || e->op != OpType::Xor || e->inputs.size() != 3)
        return false;

    bool hasXY = false;
    bool hasXZ = false;
    bool hasYZ = false;
    for (const Expr* in : e->inputs) {
        hasXY = hasXY || IsAndPair(in, x, y);
        hasXZ = hasXZ || IsAndPair(in, x, z);
        hasYZ = hasYZ || IsAndPair(in, y, z);
    }

    return hasXY && hasXZ && hasYZ;
}

bool ContainsCanonicalCH(const Expr* root, const Expr* x, const Expr* y, const Expr* z) {
    if (root == nullptr)
        return false;

    if (IsCanonicalCH(root, x, y, z))
        return true;

    for (const Expr* in : root->inputs) {
        if (ContainsCanonicalCH(in, x, y, z))
            return true;
    }

    return false;
}

bool ContainsCanonicalMAJ(const Expr* root, const Expr* x, const Expr* y, const Expr* z) {
    if (root == nullptr)
        return false;

    if (IsCanonicalMAJ(root, x, y, z))
        return true;

    for (const Expr* in : root->inputs) {
        if (ContainsCanonicalMAJ(in, x, y, z))
            return true;
    }

    return false;
}

int TestCH_HighLevelAndEquivalentForms_ConvergeToCanonical() {
    Builder b;
    auto x = b.Var();
    auto y = b.Var();
    auto z = b.Var();

    auto highLevel = b.Ch(x, y, z);
    auto lowLevelEquivalent = b.Xor({z, b.And(x, b.Xor({y, z}))});

    auto rewrittenHighLevel = MakeShaRewriteEngine().ApplyUntilStable(highLevel);
    auto rewrittenEquivalent = MakeShaRewriteEngine().ApplyUntilStable(lowLevelEquivalent);

    BF_TEST(!ContainsOp(rewrittenHighLevel, OpType::Ch));
    BF_TEST(IsCanonicalCH(rewrittenHighLevel, x, y, z));
    BF_TEST(IsCanonicalCH(rewrittenEquivalent, x, y, z));
    return 0;
}

int TestMAJ_HighLevelAndEquivalentForms_ConvergeToCanonical() {
    Builder b;
    auto x = b.Var();
    auto y = b.Var();
    auto z = b.Var();

    auto highLevel = b.Maj(x, y, z);
    auto lowLevelEquivalent = MakeOp(9001, OpType::Or, {b.And(x, y), b.And(z, b.Xor({x, y}))});

    auto rewrittenHighLevel = MakeShaRewriteEngine().ApplyUntilStable(highLevel);
    auto rewrittenEquivalent = MakeShaRewriteEngine().ApplyUntilStable(lowLevelEquivalent);

    BF_TEST(!ContainsOp(rewrittenHighLevel, OpType::Maj));
    BF_TEST(IsCanonicalMAJ(rewrittenHighLevel, x, y, z));
    BF_TEST(IsCanonicalMAJ(rewrittenEquivalent, x, y, z));
    return 0;
}

int TestRoundFragments_EmbedCanonicalCHAndMAJSubforms() {
    Builder b;

    auto e = b.Var();
    auto f = b.Var();
    auto g = b.Var();
    auto h = b.Var();
    auto chFragment = b.Add({h, b.BigSigma1(e), b.Ch(e, f, g)});
    auto rewrittenCH = MakeShaRewriteEngine().ApplyUntilStable(chFragment);

    BF_TEST(rewrittenCH->op == OpType::Add);
    BF_TEST(!ContainsOp(rewrittenCH, OpType::Ch));
    BF_TEST(ContainsCanonicalCH(rewrittenCH, e, f, g));

    auto a = b.Var();
    auto bVar = b.Var();
    auto c = b.Var();
    auto d = b.Var();
    auto majFragment = b.Add({d, b.BigSigma0(a), b.Maj(a, bVar, c)});
    auto rewrittenMAJ = MakeShaRewriteEngine().ApplyUntilStable(majFragment);

    BF_TEST(rewrittenMAJ->op == OpType::Add);
    BF_TEST(!ContainsOp(rewrittenMAJ, OpType::Maj));
    BF_TEST(ContainsCanonicalMAJ(rewrittenMAJ, a, bVar, c));
    return 0;
}

int TestSigmaFragments_XorPermutationsConvergeToSameCanonicalForm() {
    Builder b;
    auto x = b.Var();

    auto sigma0A = b.Xor({b.RotR(x, 2), b.RotR(x, 13), b.RotR(x, 22)});
    auto sigma0B = b.Xor({b.RotR(x, 22), b.RotR(x, 2), b.RotR(x, 13)});

    auto sigma1A = b.Xor({b.RotR(x, 6), b.RotR(x, 11), b.RotR(x, 25)});
    auto sigma1B = b.Xor({b.RotR(x, 25), b.RotR(x, 6), b.RotR(x, 11)});

    auto small0A = b.Xor({b.RotR(x, 7), b.RotR(x, 18), MakeOp(17000, OpType::Shr, {x, b.Const(3)})});
    auto small0B = b.Xor({MakeOp(17001, OpType::Shr, {x, b.Const(3)}), b.RotR(x, 18), b.RotR(x, 7)});

    auto small1A = b.Xor({b.RotR(x, 17), b.RotR(x, 19), MakeOp(17002, OpType::Shr, {x, b.Const(10)})});
    auto small1B = b.Xor({b.RotR(x, 19), MakeOp(17003, OpType::Shr, {x, b.Const(10)}), b.RotR(x, 17)});

    auto rSigma0A = MakeSigmaNormalizeEngine().ApplyUntilStable(sigma0A);
    auto rSigma0B = MakeSigmaNormalizeEngine().ApplyUntilStable(sigma0B);
    auto rSigma1A = MakeSigmaNormalizeEngine().ApplyUntilStable(sigma1A);
    auto rSigma1B = MakeSigmaNormalizeEngine().ApplyUntilStable(sigma1B);
    auto rSmall0A = MakeSigmaNormalizeEngine().ApplyUntilStable(small0A);
    auto rSmall0B = MakeSigmaNormalizeEngine().ApplyUntilStable(small0B);
    auto rSmall1A = MakeSigmaNormalizeEngine().ApplyUntilStable(small1A);
    auto rSmall1B = MakeSigmaNormalizeEngine().ApplyUntilStable(small1B);

    BF_TEST(ExprSignature(rSigma0A) == ExprSignature(rSigma0B));
    BF_TEST(ExprSignature(rSigma1A) == ExprSignature(rSigma1B));
    BF_TEST(ExprSignature(rSmall0A) == ExprSignature(rSmall0B));
    BF_TEST(ExprSignature(rSmall1A) == ExprSignature(rSmall1B));
    return 0;
}

int TestSigmaFragments_XorDuplicateTermsAreEliminated() {
    Builder b;
    auto x = b.Var();

    auto sigmaWithDup = b.Xor({b.RotR(x, 2), b.RotR(x, 13), b.RotR(x, 22), b.RotR(x, 13)});
    auto smallWithDup = b.Xor({b.RotR(x, 17), b.RotR(x, 19), MakeOp(17100, OpType::Shr, {x, b.Const(10)}), b.RotR(x, 19)});

    auto rSigma = MakeSigmaNormalizeEngine().ApplyUntilStable(sigmaWithDup);
    auto rSmall = MakeSigmaNormalizeEngine().ApplyUntilStable(smallWithDup);

    BF_TEST(rSigma->op == OpType::Xor);
    BF_TEST(rSigma->inputs.size() == 2);
    BF_TEST(rSmall->op == OpType::Xor);
    BF_TEST(rSmall->inputs.size() == 2);
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestCH_HighLevelAndEquivalentForms_ConvergeToCanonical);
    BF_RUN_TEST(TestMAJ_HighLevelAndEquivalentForms_ConvergeToCanonical);
    BF_RUN_TEST(TestRoundFragments_EmbedCanonicalCHAndMAJSubforms);
    BF_RUN_TEST(TestSigmaFragments_XorPermutationsConvergeToSameCanonicalForm);
    BF_RUN_TEST(TestSigmaFragments_XorDuplicateTermsAreEliminated);
    return 0;
}
