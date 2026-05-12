#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

static RuleEngine MakeArithmeticEngine() {

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.Merge(BuildSimplifyArithmetic());
    engine.Merge(BuildFactorizeArithmetic());
    return engine;
}

static bool HasCoeffBaseMul(const ExprStore* store, ExprId id, ExprId base, uint32_t coeff) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Mul)
        return false;

    bool hasBase = false;
    bool hasCoeff = false;
    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (in == base)
            hasBase = true;
        if (exprIn.op == OpType::Const && exprIn.knownValue == coeff)
            hasCoeff = true;
    }

    return hasBase && hasCoeff;
}

static bool HasCoeffBaseMul(ExprRef e, ExprRef base, uint32_t coeff) {
    if (e.store != base.store)
        return false;

    return HasCoeffBaseMul(e.store, e.id, base.id, coeff);
}

int TestLinearMultiplicity_BasicAndMixed() {
    MakeExprStore(32);

    RuleEngine engine = MakeArithmeticEngine();
    auto a = V("a");

    BF_TEST(HasCoeffBaseMul(Rewrite(engine, a + a), a, 2u));
    BF_TEST(HasCoeffBaseMul(Rewrite(engine, a + a + a), a, 3u));
    BF_TEST(HasCoeffBaseMul(Rewrite(engine, a + (a * 2)), a, 3u));
    BF_TEST(HasCoeffBaseMul(Rewrite(engine, (a * 2) + a), a, 3u));
    BF_TEST(HasCoeffBaseMul(Rewrite(engine, (a * 2) + (a * 3)), a, 5u));
    BF_TEST(HasCoeffBaseMul(Rewrite(engine, (a * 1) + (a * 2)), a, 3u));
    return 0;
}

int TestLinearMultiplicity_Guards() {
    MakeExprStore(32);

    RuleEngine engine = MakeArithmeticEngine();
    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto expr1 = a + (a * b); // symbolic coefficient -> reject
    BF_TEST(Rewrite(engine, expr1) == expr1);

    auto expr2 = (a << 1) + a; // normalize/simplify may rewrite
    BF_TEST(Rewrite(engine, expr2) != expr2);

    auto expr3 = (a * b) + (a * c); // common factor extraction
    BF_TEST(Rewrite(engine, expr3) != expr3);

    return 0;
}

int main() {
    BF_RUN_TEST(TestLinearMultiplicity_BasicAndMixed);
    BF_RUN_TEST(TestLinearMultiplicity_Guards);
    return 0;
}
