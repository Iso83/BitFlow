#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestDifferenceOfSquares_Basic() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_DifferenceOfSquares_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE(a.Pow(C(2)) - b.Pow(C(2))));
    CPPTEST_ASSERT(Op(r) == OpType::Mul);

    bool matchDiff = false;
    bool matchSum = false;

    const Expr& rExpr = store[r];
    for (BitFlow::Engine::Core::Ids::ExprId in : rExpr.inputs) {
        ExprRef inRef(&store, in);
        if (Op(inRef) == OpType::Sub)
            matchDiff = true;

        if (Op(inRef) == OpType::Add)
            matchSum = true;
    }

    CPPTEST_ASSERT(matchDiff);
    CPPTEST_ASSERT(matchSum);

    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Sub; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Add; }));

    return 0;
}

int TestDifferenceOfSquares_ShiftedTerms() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_DifferenceOfSquares_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE((C(1) + b).Pow(C(2)) - (b - C(2)).Pow(C(2))));
    CPPTEST_ASSERT(Op(r) == OpType::Mul);

    bool matchDiff = false;
    bool matchSum = false;

    const Expr& rExpr = store[r];
    for (BitFlow::Engine::Core::Ids::ExprId in : rExpr.inputs) {
        ExprRef inRef(&store, in);
        if (Op(inRef) == OpType::Sub)
            matchDiff = true;

        if (Op(inRef) == OpType::Add)
            matchSum = true;
    }

    CPPTEST_ASSERT(matchDiff);
    CPPTEST_ASSERT(matchSum);

    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Sub; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Add; }));

    return 0;
}

int TestDifferenceOfSquares_NoMatch() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_DifferenceOfSquares_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r1, BF_REWRITE(a.Pow(C(3)) - b.Pow(C(2))));
    CPPTEST_ASSERT(Op(r1) != OpType::Mul);

    BF_SAFE_REWRITE(r2, BF_REWRITE(a.Pow(C(2)) - b.Pow(C(3))));
    CPPTEST_ASSERT(Op(r2) != OpType::Mul);

    BF_SAFE_REWRITE(r3, BF_REWRITE(a.Pow(C(2)) - b));
    CPPTEST_ASSERT(Op(r3) != OpType::Mul);

    return 0;
}

int main() {
    CPPTEST_RUN(TestDifferenceOfSquares_Basic);
    CPPTEST_RUN(TestDifferenceOfSquares_ShiftedTerms);
    CPPTEST_RUN(TestDifferenceOfSquares_NoMatch);
    return 0;
}
