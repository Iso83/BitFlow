#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Core::Testing;
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

int TestSimplify_NegNeg() {
    MakeExprStore(32);

    RuleEngine engine = MakeArithmeticEngine();
    auto x = V("x");

    BF_TEST(Rewrite(engine, -(-x)) == x);
    return 0;
}

int TestSimplify_ConstCombine_Basic() {
    MakeExprStore(32);

    RuleEngine engine = MakeArithmeticEngine();

    BF_TEST(EqualChunkValue(Rewrite(engine, C(5) + 7), 12u));
    BF_TEST(EqualChunkValue(Rewrite(engine, C(9) - 4), 5u));
    BF_TEST(EqualChunkValue(Rewrite(engine, C(3) * 6), 18u));
    BF_TEST(EqualChunkValue(Rewrite(engine, C(8) / 2), 4u));
    return 0;
}

int TestModZero_Guard_Preserved() {
    MakeExprStore(32);

    RuleEngine engine = MakeArithmeticEngine();
    auto x = V("x");

    bool thrown = false;
    try {
        Rewrite(engine, x % 0);
    } catch (const std::exception&) {
        thrown = true;
    }

    BF_TEST(thrown);
    return 0;
}

int TestFactorize_AddCommonFactor() {
    MakeExprStore(32);

    RuleEngine engine = MakeArithmeticEngine();
    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto r = Rewrite(engine, (a * b) + (a * c));
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::Mul);
    BF_TEST(out.inputs.size() == 2);
    BF_TEST(CountExpr(r, a) == 1);
    return 0;
}

int TestFactorize_Canonical_a_b_plus_b_a() {
    MakeExprStore(32);

    RuleEngine engine = MakeArithmeticEngine();
    auto a = V("a");
    auto b = V("b");
    auto r = Rewrite(engine, (a * b) + (b * a));
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::Mul);
    BF_TEST(CountExpr(r, a) == 1);
    BF_TEST(CountExpr(r, b) == 1);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 2u); }));

    return 0;
}

int TestFactorize_AddRepeatedTermCount() {
    MakeExprStore(32);

    RuleEngine engine = MakeArithmeticEngine();
    auto a = V("a");
    auto term = a * 2;
    auto r = Rewrite(engine, term + term + term);
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::Mul);
    BF_TEST(out.inputs.size() == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 6u); }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    return 0;
}

int TestFactorize_CombineNestedMulConstants() {
    MakeExprStore(32);

    RuleEngine engine = MakeArithmeticEngine();
    auto a = V("a");
    auto r = Rewrite(engine, C(3) * (a * 2));
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::Mul);
    BF_TEST(out.inputs.size() == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 6u); }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    return 0;
}

int TestFactorize_AddLinearMultiplicityMixedForms() {
    MakeExprStore(32);

    RuleEngine engine = MakeArithmeticEngine();
    auto a = V("a");
    auto b = V("b");

    {
        auto r = Rewrite(engine, a + a);
        auto out = GetExpr(r);

        BF_TEST(out.op == OpType::Mul);
        BF_TEST(out.inputs.size() == 2);
        BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 2u); }));
        BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    }

    {
        auto r = Rewrite(engine, a + a + a);
        auto out = GetExpr(r);

        BF_TEST(out.op == OpType::Mul);
        BF_TEST(out.inputs.size() == 2);
        BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 3u); }));
        BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    }

    {
        auto r = Rewrite(engine, a + (a * 2));
        auto out = GetExpr(r);

        BF_TEST(out.op == OpType::Mul);
        BF_TEST(out.inputs.size() == 2);
        BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 3u); }));
        BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    }

    {
        auto r = Rewrite(engine, a + (C(2) * a));
        auto out = GetExpr(r);

        BF_TEST(out.op == OpType::Mul);
        BF_TEST(out.inputs.size() == 2);
        BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 3u); }));
        BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    }

    {
        auto r = Rewrite(engine, (a * 2) + (a * 3));
        auto out = GetExpr(r);

        BF_TEST(out.op == OpType::Mul);
        BF_TEST(out.inputs.size() == 2);
        BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 5u); }));
        BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    }

    {
        auto r = Rewrite(engine, b + a + a);
        auto out = GetExpr(r);

        BF_TEST(out.op == OpType::Mul);
        BF_TEST(out.inputs.size() == 2);
        BF_TEST(AnyInput(r, [&](ExprRef in) { return in == b; }));
        BF_TEST(AnyInput(r, [&](ExprRef in) {
            if (GetExpr(in).op != OpType::Mul)
                return false;

            return AnyInput(in, [&](ExprRef factor) { return factor == a; }) &&
                   AnyInput(in, [&](ExprRef factor) { return EqualChunkValue(factor, 2u); });
        }));
    }

    {
        auto r = Rewrite(engine, (a * 1) + (a * 2));
        auto out = GetExpr(r);

        BF_TEST(out.op == OpType::Mul);
        BF_TEST(out.inputs.size() == 2);
        BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 3u); }));
        BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    }

    return 0;
}

int TestFactorize_AddLinearMultiplicity_Guards() {
    MakeExprStore(32);

    RuleEngine engine = MakeArithmeticEngine();
    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    {
        auto expr = a + (a * b);
        BF_TEST(Rewrite(engine, expr) == expr);
    }

    {
        auto expr = (a << 1) + a;
        BF_TEST(Rewrite(engine, expr) == expr);
    }

    {
        auto expr = a * (b + c);
        BF_TEST(Rewrite(engine, expr) == expr);
    }

    return 0;
}

int main() {
    BF_RUN_TEST(TestSimplify_NegNeg);
    BF_RUN_TEST(TestSimplify_ConstCombine_Basic);
    BF_RUN_TEST(TestModZero_Guard_Preserved);
    BF_RUN_TEST(TestFactorize_AddCommonFactor);
    BF_RUN_TEST(TestFactorize_Canonical_a_b_plus_b_a);
    BF_RUN_TEST(TestFactorize_AddRepeatedTermCount);
    BF_RUN_TEST(TestFactorize_CombineNestedMulConstants);
    BF_RUN_TEST(TestFactorize_AddLinearMultiplicityMixedForms);
    BF_RUN_TEST(TestFactorize_AddLinearMultiplicity_Guards);
    return 0;
}
