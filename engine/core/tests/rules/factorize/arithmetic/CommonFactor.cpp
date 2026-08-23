#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

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

    BF_SAFE_REWRITE(r, BF_REWRITE(a + a));

    CPPTEST_ASSERT(Op(r) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(EqualChunkValue(Input(r, 0), 2u));
    CPPTEST_ASSERT(Input(r, 1) == a);
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

    BF_SAFE_REWRITE(r, BF_REWRITE((a * 2) + a));

    CPPTEST_ASSERT(Op(r) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(EqualChunkValue(Input(r, 0), 3u));
    CPPTEST_ASSERT(Input(r, 1) == a);
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

    BF_SAFE_REWRITE(r, BF_REWRITE(a + (a * 2) + (a * 3)));

    CPPTEST_ASSERT(Op(r) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(EqualChunkValue(Input(r, 0), 6u));
    CPPTEST_ASSERT(Input(r, 1) == a);
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

    BF_SAFE_REWRITE(r, BF_REWRITE(a + (a * 2) + b));

    CPPTEST_ASSERT(Op(r) == OpType::Add);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == b; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Mul && HasCoeffBaseMul(in, a, 3u); }));
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

    BF_SAFE_REWRITE(r, BF_REWRITE(a - C(4) + a + a));

    CPPTEST_ASSERT(Op(r) == OpType::Sub);
    CPPTEST_ASSERT(InputSize(r) == 2);

    ExprRef lhs = Input(r, 0);
    ExprRef rhs = Input(r, 1);

    // rhs => 4
    CPPTEST_ASSERT(Op(rhs) == OpType::Const);
    CPPTEST_ASSERT(ExprOf(rhs).knownValue == 4);

    // lhs => 3 * a
    CPPTEST_ASSERT(Op(lhs) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(lhs) == 2);
    CPPTEST_ASSERT(CountInput(lhs, a) == 1);
    CPPTEST_ASSERT(CountInputsIf(lhs, [](ExprRef x) { return Op(x) == OpType::Const; }) == 1);
    CPPTEST_ASSERT(AnyInput(lhs, [](ExprRef x) { return Op(x) == OpType::Const && ExprOf(x).knownValue == 3; }));

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

    BF_SAFE_REWRITE(r, BF_REWRITE((a * b) + (a * c)));

    CPPTEST_ASSERT(Op(r) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(CountInput(r, a) == 1);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Add; }));
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

    BF_SAFE_REWRITE(r, BF_REWRITE((a * b) + (b * a)));

    CPPTEST_ASSERT(Op(r) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(r) == 3);
    CPPTEST_ASSERT(CountInput(r, a) == 1);
    CPPTEST_ASSERT(CountInput(r, b) == 1);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 2u); }));
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

    BF_SAFE_REWRITE(r, BF_REWRITE((a * b) + (a * c) + c));

    CPPTEST_ASSERT(Op(r) == OpType::Add);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == c; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Mul && CountInput(in, a) == 1; }));
    return 0;
}

static bool IsMulOf(ExprRef e, std::initializer_list<ExprRef> expectedFactors) {
    if (Op(e) != OpType::Mul || InputSize(e) != expectedFactors.size())
        return false;

    for (ExprRef expected : expectedFactors) {
        if (!AnyInput(e, [&](ExprRef in) { return in == expected; }))
            return false;
    }

    return true;
}

int TestPromoteFactorsToPower_Basic() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_PromoteFactorsToPower_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto ab = a * b;

    BF_SAFE_REWRITE(r, BF_REWRITE(a * b * c * ab.Pow(C(2))));

    CPPTEST_ASSERT(Op(r) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == c; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        return Op(in) == OpType::Pow && IsMulOf(Input(in, 0), {a, b}) && EqualChunkValue(Input(in, 1), 3u);
    }));
    return 0;
}

int TestPromoteFactorsToPower_LargerBase() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_PromoteFactorsToPower_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");
    auto abc = a * b * c;

    BF_SAFE_REWRITE(r, BF_REWRITE(a * b * c * d * abc.Pow(C(5))));

    CPPTEST_ASSERT(Op(r) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == d; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        return Op(in) == OpType::Pow && IsMulOf(Input(in, 0), {a, b, c}) && EqualChunkValue(Input(in, 1), 6u);
    }));
    return 0;
}

int TestPromoteFactorsToPower_NoPartialMatch() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_PromoteFactorsToPower_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto ab = a * b;
    auto expr = a * c * ab.Pow(C(2));

    BF_SAFE_REWRITE(r, BF_REWRITE(expr));

    CPPTEST_ASSERT(Op(r) == OpType::Mul);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == a; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == c; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        return Op(in) == OpType::Pow && IsMulOf(Input(in, 0), {a, b}) && EqualChunkValue(Input(in, 1), 2u);
    }));
    CPPTEST_ASSERT(!AnyInput(r, [&](ExprRef in) {
        return Op(in) == OpType::Pow && IsMulOf(Input(in, 0), {a, b}) && EqualChunkValue(Input(in, 1), 3u);
    }));
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

    BF_SAFE_REWRITE(r, BF_REWRITE(((a.Pow(5) * C(2)) / (C(3) * a.Pow(5)))));

    CPPTEST_ASSERT(Op(r) == OpType::Div);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(Input(r, 0), [](ExprRef in) { return EqualChunkValue(in, 2u); }) ||
                   EqualChunkValue(Input(r, 0), 2u));
    CPPTEST_ASSERT(AnyInput(Input(r, 1), [](ExprRef in) { return EqualChunkValue(in, 3u); }) ||
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

    // (a**8 * 2) / (3 * a**5)
    // => (2 * a**3) / 3

    BF_SAFE_REWRITE(r, BF_REWRITE(((a.Pow(8) * C(2)) / (C(3) * a.Pow(5)))));

    CPPTEST_ASSERT(Op(r) == OpType::Div);
    CPPTEST_ASSERT(InputSize(r) == 2);

    const ExprRef lhs = Input(r, 0);
    const ExprRef rhs = Input(r, 1);

    // denominator should be exactly 3
    CPPTEST_ASSERT(EqualChunkValue(rhs, 3u));

    // lhs should be multiplication
    CPPTEST_ASSERT(Op(lhs) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(lhs) == 2);

    // old powers must be gone
    CPPTEST_ASSERT(CountInput(r, a.Pow(8)) == 0);
    CPPTEST_ASSERT(CountInput(r, a.Pow(5)) == 0);

    // lhs should contain constant 2
    CPPTEST_ASSERT(AnyInput(lhs, [](ExprRef in) { return EqualChunkValue(in, 2u); }));

    // lhs should contain a**3
    CPPTEST_ASSERT(AnyInput(lhs, [&](ExprRef in) { return IsPow(in, a, 3u); }));

    return 0;
}

int TestCommonFactorCancel_PowTerms_DirectDivision() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_CommonFactorCancel_PowTerms_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(a.Pow(8) / a.Pow(5)));

    CPPTEST_ASSERT(IsPow(r, a, 3u));
    return 0;
}

int TestSubCommonDenominator() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_SubCommonDenominator_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    BF_SAFE_REWRITE(r, BF_REWRITE(C(7) / C(8) - (C(5) / C(8))));

    CPPTEST_ASSERT(Op(r) == OpType::Div);

    ExprRef lhs = Input(r, 0);
    ExprRef rhs = Input(r, 1);

    CPPTEST_ASSERT(Op(lhs) == OpType::Sub);
    CPPTEST_ASSERT(EqualChunkValue(rhs, 8u));
    CPPTEST_ASSERT(EqualChunkValue(Input(lhs, 0), 7u));
    CPPTEST_ASSERT(EqualChunkValue(Input(lhs, 1), 5u));

    return 0;
}

int TestSubCommonDenominator_Variables() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_SubCommonDenominator_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE((a / c) - (b / c)));

    CPPTEST_ASSERT(Op(r) == OpType::Div);

    ExprRef lhs = Input(r, 0);
    ExprRef rhs = Input(r, 1);

    CPPTEST_ASSERT(Op(lhs) == OpType::Sub);

    CPPTEST_ASSERT(rhs == c);
    CPPTEST_ASSERT(Input(lhs, 0) == a);
    CPPTEST_ASSERT(Input(lhs, 1) == b);

    return 0;
}

int TestSubCommonDenominator_DifferentDenominator_NoRewrite() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_SubCommonDenominator_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");

    auto expr = (a / c) - (b / d);
    BF_SAFE_REWRITE(r, BF_REWRITE(expr));

    CPPTEST_ASSERT(r == expr);

    return 0;
}

int TestSubCommonDenominator_ComplexNumerator() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_SubCommonDenominator_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");
    auto x = V("x");

    auto term1 = (a + b);
    auto term2 = (c * d);

    BF_SAFE_REWRITE(r, BF_REWRITE((term1 / x) - (term2 / x)));

    CPPTEST_ASSERT(Op(r) == OpType::Div);

    ExprRef sub = Input(r, 0);

    CPPTEST_ASSERT(Op(sub) == OpType::Sub);

    CPPTEST_ASSERT(Input(sub, 0) == term1);
    CPPTEST_ASSERT(Input(sub, 1) == term2);

    return 0;
}

int TestSubCommonDenominator_WithLeadingAddTerm() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_SubCommonDenominator_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    BF_SAFE_REWRITE(r, BF_REWRITE((C(40) + (C(5) / C(8))) - (C(3) / C(8))));

    CPPTEST_ASSERT(Op(r) == OpType::Add);
    CPPTEST_ASSERT(AnyInput(r, [](ExprRef in) { return EqualChunkValue(in, 40u); }));

    CPPTEST_ASSERT(AnyInput(r, [](ExprRef in) {
        if (Op(in) != OpType::Div)
            return false;
        if (Op(Input(in, 0)) != OpType::Sub)
            return false;
        return EqualChunkValue(Input(Input(in, 0), 0), 5u) && EqualChunkValue(Input(Input(in, 0), 1), 3u) &&
               EqualChunkValue(Input(in, 1), 8u);
    }));

    return 0;
}

int TestAddCommonDenominator() {
    MakeExprStore(32);

    const auto rule = Factorize::Arithmetic::Get_AddCommonDenominator_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto x = V("x");

    BF_SAFE_REWRITE(r, BF_REWRITE((a / x) + (b / x)));

    CPPTEST_ASSERT(Op(r) == OpType::Div);

    ExprRef numerator = Input(r, 0);
    ExprRef denominator = Input(r, 1);

    CPPTEST_ASSERT(Op(numerator) == OpType::Add);

    CPPTEST_ASSERT(AnyInput(numerator, [&](ExprRef in) { return in == a; }));
    CPPTEST_ASSERT(AnyInput(numerator, [&](ExprRef in) { return in == b; }));

    CPPTEST_ASSERT(denominator == x);

    return 0;
}

int TestCommonFactorCancel_DivisionFactors() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_CommonFactorCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE((a * b * C(2)) / (b * C(3))));

    CPPTEST_ASSERT(Op(r) == OpType::Div);
    CPPTEST_ASSERT(InputSize(r) == 2);

    const ExprRef numerator = Input(r, 0);
    const ExprRef denominator = Input(r, 1);

    CPPTEST_ASSERT(Op(numerator) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(numerator) == 2);
    CPPTEST_ASSERT(AnyInput(numerator, [](ExprRef in) { return EqualChunkValue(in, 2u); }));
    CPPTEST_ASSERT(AnyInput(numerator, [&](ExprRef in) { return in == a; }));
    CPPTEST_ASSERT(EqualChunkValue(denominator, 3u));
    CPPTEST_ASSERT(CountInput(r, b) == 0);

    return 0;
}

int TestCommonFactorCancel_SubDivMul() {
    MakeExprStore(32);
    const auto rule = Factorize::Arithmetic::Get_CommonFactorCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    BF_SAFE_REWRITE(r, BF_REWRITE(C(5) / C(8) - ((C(3) / C(8)) * C(8))));

    CPPTEST_ASSERT(Op(r) == OpType::Sub);
    CPPTEST_ASSERT(Op(Input(r, 0)) == OpType::Div);
    CPPTEST_ASSERT(EqualChunkValue(Input(Input(r, 0), 0), 5u));
    CPPTEST_ASSERT(EqualChunkValue(Input(Input(r, 0), 1), 8u));
    CPPTEST_ASSERT(EqualChunkValue(Input(r, 1), 3u));

    return 0;
}

int main() {
    CPPTEST_RUN(TestAddLinearMultiplicity_Basic);
    CPPTEST_RUN(TestAddLinearMultiplicity_ImplicitAndExplicitCoeff);
    CPPTEST_RUN(TestAddLinearMultiplicity_MergesMultipleTerms);
    CPPTEST_RUN(TestAddLinearMultiplicity_PreservesPassthroughTerms);
    CPPTEST_RUN(TestAddLinearMultiplicity_Chain);

    CPPTEST_RUN(TestAddCommonFactor_Basic);
    CPPTEST_RUN(TestAddCommonFactor_CommutativeMulOperands);
    CPPTEST_RUN(TestAddCommonFactor_PartialFactorization);
    CPPTEST_RUN(TestPromoteFactorsToPower_Basic);
    CPPTEST_RUN(TestPromoteFactorsToPower_LargerBase);
    CPPTEST_RUN(TestPromoteFactorsToPower_NoPartialMatch);
    CPPTEST_RUN(TestCommonFactorCancel_PowTerms_Basic);
    CPPTEST_RUN(TestCommonFactorCancel_PowTerms_ExponentDifference);
    CPPTEST_RUN(TestCommonFactorCancel_PowTerms_DirectDivision);

    CPPTEST_RUN(TestSubCommonDenominator);
    CPPTEST_RUN(TestSubCommonDenominator_Variables);
    CPPTEST_RUN(TestSubCommonDenominator_DifferentDenominator_NoRewrite);
    CPPTEST_RUN(TestSubCommonDenominator_ComplexNumerator);
    CPPTEST_RUN(TestSubCommonDenominator_WithLeadingAddTerm);
    CPPTEST_RUN(TestAddCommonDenominator);
    CPPTEST_RUN(TestCommonFactorCancel_DivisionFactors);
    CPPTEST_RUN(TestCommonFactorCancel_SubDivMul);
    return 0;
}
