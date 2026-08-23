#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestNegNeg_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_NegNeg_Rule();

    RuleEngine engine;
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(-(-a)));

    CPPTEST_ASSERT(r == a);
    return 0;
}

int TestNegNeg_TripleNeg() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_NegNeg_Rule();

    RuleEngine engine;
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(-(-(-a))));

    CPPTEST_ASSERT(Op(r) == OpType::Neg);
    CPPTEST_ASSERT(InputSize(r) == 1);
    CPPTEST_ASSERT(Input(r, 0) == a);

    return 0;
}

int TestNegNeg_NoMatchSingleNeg() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_NegNeg_Rule();

    RuleEngine engine;
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(-a));

    CPPTEST_ASSERT(Op(r) == OpType::Neg);
    CPPTEST_ASSERT(InputSize(r) == 1);
    CPPTEST_ASSERT(Input(r, 0) == a);

    return 0;
}

int main() {
    CPPTEST_RUN(TestNegNeg_Basic);
    CPPTEST_RUN(TestNegNeg_TripleNeg);
    CPPTEST_RUN(TestNegNeg_NoMatchSingleNeg);
    return 0;
}
