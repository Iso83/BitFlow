#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestNegNeg_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_NegNeg_Rule();

    RuleEngine engine;
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, Rewrite(engine, -(-a)));

    BF_TEST(r == a);
    return 0;
}

int TestNegNeg_TripleNeg() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_NegNeg_Rule();

    RuleEngine engine;
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, Rewrite(engine, -(-(-a))));

    BF_TEST(Op(r) == OpType::Neg);
    BF_TEST(InputSize(r) == 1);
    BF_TEST(Input(r, 0) == a);

    return 0;
}

int TestNegNeg_NoMatchSingleNeg() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_NegNeg_Rule();

    RuleEngine engine;
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, Rewrite(engine, -a));

    BF_TEST(Op(r) == OpType::Neg);
    BF_TEST(InputSize(r) == 1);
    BF_TEST(Input(r, 0) == a);

    return 0;
}

int main() {
    BF_RUN_TEST(TestNegNeg_Basic);
    BF_RUN_TEST(TestNegNeg_TripleNeg);
    BF_RUN_TEST(TestNegNeg_NoMatchSingleNeg);
    return 0;
}