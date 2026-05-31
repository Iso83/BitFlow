#include <ExprTestUtils.h>
#include <RuleTestUtils.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestSubToNeg_RewritesWhenOutOfOrder() {
    MakeExprStore(32);
    const auto rule = Normalize::Arithmetic::Get_SubToNeg_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    BF_SAFE_REWRITE(r, BF_REWRITE(C(1) - a));

    BF_TEST(Op(r) == OpType::Neg);
    BF_TEST(Op(Input(r, 0)) == OpType::Sub);
    BF_TEST(Input(Input(r, 0), 0) == a);
    BF_TEST(EqualChunkValue(Input(Input(r, 0), 1), 1u));
    return 0;
}

int TestSubToNeg_NoRewriteWhenCanonical() {
    MakeExprStore(32);
    const auto rule = Normalize::Arithmetic::Get_SubToNeg_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto expr = a - C(1);
    BF_SAFE_REWRITE(r, BF_REWRITE(expr));
    BF_TEST(r == expr);
    return 0;
}

int TestSubToNeg_NoRewriteWhenLhsNotConst() {
    MakeExprStore(32);
    const auto rule = Normalize::Arithmetic::Get_SubToNeg_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto expr = a - b;
    BF_SAFE_REWRITE(r, BF_REWRITE(expr));
    BF_TEST(r == expr);
    return 0;
}

int TestSubToNeg_NoRewriteWhenRhsConst() {
    MakeExprStore(32);
    const auto rule = Normalize::Arithmetic::Get_SubToNeg_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto expr = a - C(2);
    BF_SAFE_REWRITE(r, BF_REWRITE(expr));
    BF_TEST(r == expr);
    return 0;
}

int TestSubToNeg_RewriteCompositeRhs() {
    MakeExprStore(32);
    const auto rule = Normalize::Arithmetic::Get_SubToNeg_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE(C(1) - (a + b)));

    BF_TEST(Op(r) == OpType::Neg);

    ExprRef sub = Input(r, 0);

    BF_TEST(Op(sub) == OpType::Sub);
    BF_TEST(AnyInput(sub, [&](ExprRef x) { return Op(x) == OpType::Add; }));
    BF_TEST(AnyInput(sub, [&](ExprRef x) { return Op(x) == OpType::Const && ExprOf(x).knownValue == 1; }));

    return 0;
}

int main() {
    BF_RUN_TEST(TestSubToNeg_RewritesWhenOutOfOrder);
    BF_RUN_TEST(TestSubToNeg_NoRewriteWhenCanonical);
    BF_RUN_TEST(TestSubToNeg_NoRewriteWhenLhsNotConst);
    BF_RUN_TEST(TestSubToNeg_NoRewriteWhenRhsConst);
    BF_RUN_TEST(TestSubToNeg_RewriteCompositeRhs);
    return 0;
}
