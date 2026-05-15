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
    BF_TEST(Input(r, 0) == a);
    BF_TEST(EqualChunkValue(Input(r, 1), 2u));
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
    BF_TEST(Input(r, 0) == a);
    BF_TEST(EqualChunkValue(Input(r, 1), 3u));
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
    BF_TEST(Input(r, 0) == a);
    BF_TEST(EqualChunkValue(Input(r, 1), 6u));
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

int main() {
    BF_RUN_TEST(TestAddLinearMultiplicity_Basic);
    BF_RUN_TEST(TestAddLinearMultiplicity_ImplicitAndExplicitCoeff);
    BF_RUN_TEST(TestAddLinearMultiplicity_MergesMultipleTerms);
    BF_RUN_TEST(TestAddLinearMultiplicity_PreservesPassthroughTerms);

    BF_RUN_TEST(TestAddCommonFactor_Basic);
    BF_RUN_TEST(TestAddCommonFactor_CommutativeMulOperands);
    BF_RUN_TEST(TestAddCommonFactor_PartialFactorization);

    return 0;
}