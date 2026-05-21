#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

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

int TestAddLinearMultiplicity_Basic() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_AddCommonFactor_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Factorize::Arithmetic::Get_AddLinearMultiplicity_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, Rewrite(engine, a + a));

    BF_TEST(Op(r) == OpType::Mul);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(EqualChunkValue(Input(r, 0), 2u));
    BF_TEST(Input(r, 1) == a);
    return 0;
}

int TestAddLinearMultiplicity_ImplicitAndExplicitCoeff() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_AddLinearMultiplicity_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, Rewrite(engine, (a * 2) + a));

    BF_TEST(Op(r) == OpType::Mul);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(EqualChunkValue(Input(r, 0), 3u));
    BF_TEST(Input(r, 1) == a);
    return 0;
}

int TestAddLinearMultiplicity_MergesMultipleTerms() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_AddLinearMultiplicity_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, Rewrite(engine, a + (a * 2) + (a * 3)));

    BF_TEST(Op(r) == OpType::Mul);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(EqualChunkValue(Input(r, 0), 6u));
    BF_TEST(Input(r, 1) == a);
    return 0;
}

int TestAddLinearMultiplicity_PreservesPassthroughTerms() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_AddLinearMultiplicity_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, Rewrite(engine, a + (a * 2) + b));

    BF_TEST(Op(r) == OpType::Add);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == b; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Mul && HasCoeffBaseMul(in, a, 3u); }));
    return 0;
}

static int TestAddLinearMultiplicity_Chain() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_AddLinearMultiplicity_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, Rewrite(engine, a - C(4) + a + a));

    BF_TEST(Op(r) == OpType::Sub);
    BF_TEST(InputSize(r) == 2);

    ExprRef lhs = Input(r, 0);
    ExprRef rhs = Input(r, 1);

    // rhs => 4
    BF_TEST(Op(rhs) == OpType::Const);
    BF_TEST(ExprOf(rhs).knownValue == 4);

    // lhs => 3 * a
    BF_TEST(Op(lhs) == OpType::Mul);
    BF_TEST(InputSize(lhs) == 2);
    BF_TEST(CountExpr(lhs, a) == 1);
    BF_TEST(CountInput(lhs, [](ExprRef x) { return Op(x) == OpType::Const; }) == 1);
    BF_TEST(AnyInput(lhs, [](ExprRef x) { return Op(x) == OpType::Const && ExprOf(x).knownValue == 3; }));

    return 0;
}

int TestAddCommonFactor_Basic() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_AddCommonFactor_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Factorize::Arithmetic::Get_AddLinearMultiplicity_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, Rewrite(engine, (a * b) + (a * c)));

    BF_TEST(Op(r) == OpType::Mul);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(CountExpr(r, a) == 1);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Add; }));
    return 0;
}

int TestAddCommonFactor_CommutativeMulOperands() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_AddCommonFactor_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Factorize::Arithmetic::Get_AddLinearMultiplicity_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, Rewrite(engine, (a * b) + (b * a)));

    BF_TEST(Op(r) == OpType::Mul);
    BF_TEST(InputSize(r) == 3);
    BF_TEST(CountExpr(r, a) == 1);
    BF_TEST(CountExpr(r, b) == 1);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 2u); }));
    return 0;
}

int TestAddCommonFactor_PartialFactorization() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_AddCommonFactor_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Factorize::Arithmetic::Get_AddLinearMultiplicity_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, Rewrite(engine, (a * b) + (a * c) + c));

    BF_TEST(Op(r) == OpType::Add);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == c; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Mul && CountExpr(in, a) == 1; }));
    return 0;
}

int TestCommonFactorCancel_PowTerms_Basic() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_CommonFactorCancel_PowTerms_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, Rewrite(engine, ((a.Pow(5) * C(2)) / (C(3) * a.Pow(5)))));

    BF_TEST(Op(r) == OpType::Div);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(Input(r, 0), [](ExprRef in) { return EqualChunkValue(in, 2u); }) ||
            EqualChunkValue(Input(r, 0), 2u));
    BF_TEST(AnyInput(Input(r, 1), [](ExprRef in) { return EqualChunkValue(in, 3u); }) ||
            EqualChunkValue(Input(r, 1), 3u));
    return 0;
}

int TestCommonFactorCancel_PowTerms_ExponentDifference() {
    MakeExprStore(32);

    const auto rule = Factorize::Arithmetic::Get_CommonFactorCancel_PowTerms_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto exp3 = C(3);

    // (a^8 * 2) / (3 * a^5)
    // => (2 * a^3) / 3

    BF_SAFE_REWRITE(r, Rewrite(engine, ((a.Pow(8) * C(2)) / (C(3) * a.Pow(5)))));

    BF_TEST(Op(r) == OpType::Div);
    BF_TEST(InputSize(r) == 2);

    const ExprRef lhs = Input(r, 0);
    const ExprRef rhs = Input(r, 1);

    // denominator should be exactly 3
    BF_TEST(EqualChunkValue(rhs, 3u));

    // lhs should be multiplication
    BF_TEST(Op(lhs) == OpType::Mul);
    BF_TEST(InputSize(lhs) == 2);

    // old powers must be gone
    BF_TEST(CountExpr(r, a.Pow(8)) == 0);
    BF_TEST(CountExpr(r, a.Pow(5)) == 0);

    // lhs should contain constant 2
    BF_TEST(AnyInput(lhs, [](ExprRef in) { return EqualChunkValue(in, 2u); }));

    // lhs should contain a^3
    BF_TEST(AnyInput(lhs, [&](ExprRef in) { return IsPow(in, a, 3u); }));

    return 0;
}

int main() {
    BF_RUN_TEST(TestAddLinearMultiplicity_Basic);
    BF_RUN_TEST(TestAddLinearMultiplicity_ImplicitAndExplicitCoeff);
    BF_RUN_TEST(TestAddLinearMultiplicity_MergesMultipleTerms);
    BF_RUN_TEST(TestAddLinearMultiplicity_PreservesPassthroughTerms);
    BF_RUN_TEST(TestAddLinearMultiplicity_Chain);

    BF_RUN_TEST(TestAddCommonFactor_Basic);
    BF_RUN_TEST(TestAddCommonFactor_CommutativeMulOperands);
    BF_RUN_TEST(TestAddCommonFactor_PartialFactorization);
    BF_RUN_TEST(TestCommonFactorCancel_PowTerms_Basic);
    BF_RUN_TEST(TestCommonFactorCancel_PowTerms_ExponentDifference);

    return 0;
}
