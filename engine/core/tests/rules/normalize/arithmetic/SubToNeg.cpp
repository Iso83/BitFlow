#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestSubToNeg_RewritesWhenOutOfOrder() {
    MakeExprStore(32);
    const auto rule = Normalize::Arithmetic::Get_SubToNeg_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    BF_SAFE_REWRITE(r, BF_REWRITE(C(1) - a));

    CPPTEST_ASSERT(Op(r) == OpType::Neg);
    CPPTEST_ASSERT(Op(Input(r, 0)) == OpType::Sub);
    CPPTEST_ASSERT(Input(Input(r, 0), 0) == a);
    CPPTEST_ASSERT(EqualChunkValue(Input(Input(r, 0), 1), 1u));
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
    CPPTEST_ASSERT(r == expr);
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
    CPPTEST_ASSERT(r == expr);
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
    CPPTEST_ASSERT(r == expr);
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

    CPPTEST_ASSERT(Op(r) == OpType::Neg);

    ExprRef sub = Input(r, 0);

    CPPTEST_ASSERT(Op(sub) == OpType::Sub);
    CPPTEST_ASSERT(AnyInput(sub, [&](ExprRef x) { return Op(x) == OpType::Add; }));
    CPPTEST_ASSERT(AnyInput(sub, [&](ExprRef x) { return Op(x) == OpType::Const && ExprOf(x).knownValue == 1; }));

    return 0;
}

int main() {
    CPPTEST_RUN(TestSubToNeg_RewritesWhenOutOfOrder);
    CPPTEST_RUN(TestSubToNeg_NoRewriteWhenCanonical);
    CPPTEST_RUN(TestSubToNeg_NoRewriteWhenLhsNotConst);
    CPPTEST_RUN(TestSubToNeg_NoRewriteWhenRhsConst);
    CPPTEST_RUN(TestSubToNeg_RewriteCompositeRhs);
    return 0;
}
