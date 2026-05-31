#include <ExprTestUtils.h>
#include <RuleTestUtils.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

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
    BF_TEST(Op(r) == OpType::Mul);

    bool matchDiff = false;
    bool matchSum = false;

    const Expr& rExpr = store[r];
    for (BitFlow::Core::Ids::ExprId in : rExpr.inputs) {
        ExprRef inRef(&store, in);
        if (Op(inRef) == OpType::Sub)
            matchDiff = true;

        if (Op(inRef) == OpType::Add)
            matchSum = true;
    }

    BF_TEST(matchDiff);
    BF_TEST(matchSum);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Sub; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Add; }));

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
    BF_TEST(Op(r) == OpType::Mul);

    bool matchDiff = false;
    bool matchSum = false;

    const Expr& rExpr = store[r];
    for (BitFlow::Core::Ids::ExprId in : rExpr.inputs) {
        ExprRef inRef(&store, in);
        if (Op(inRef) == OpType::Sub)
            matchDiff = true;

        if (Op(inRef) == OpType::Add)
            matchSum = true;
    }

    BF_TEST(matchDiff);
    BF_TEST(matchSum);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Sub; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Add; }));

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
    BF_TEST(Op(r1) != OpType::Mul);

    BF_SAFE_REWRITE(r2, BF_REWRITE(a.Pow(C(2)) - b.Pow(C(3))));
    BF_TEST(Op(r2) != OpType::Mul);

    BF_SAFE_REWRITE(r3, BF_REWRITE(a.Pow(C(2)) - b));
    BF_TEST(Op(r3) != OpType::Mul);

    return 0;
}

int main() {
    BF_RUN_TEST(TestDifferenceOfSquares_Basic);
    BF_RUN_TEST(TestDifferenceOfSquares_ShiftedTerms);
    BF_RUN_TEST(TestDifferenceOfSquares_NoMatch);
    return 0;
}
