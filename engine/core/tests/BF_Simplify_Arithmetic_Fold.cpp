#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestAddFold() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_AddFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x + 10 + 20));

    BF_TEST(Op(r) == OpType::Add);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 30u); }));
    return 0;
}

int TestSubConstFold() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubConstFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, (x + 8) - 1));

    BF_TEST(Op(r) == OpType::Add);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 7u); }));
    return 0;
}

int TestSubConstFold_MultiConst() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubConstFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, (x + 8 + 2) - 1));

    BF_TEST(Op(r) == OpType::Add);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 9u); }));
    return 0;
}

int TestSubConstFold_Cancel() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubConstFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, (x + 1) - 1));

    BF_TEST(r == x);
    return 0;
}

int TestSubAddSelfCancel() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubAddSelfCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r, Rewrite(engine, x + y - x));

    BF_TEST(r == y);
    return 0;
}

int TestSubAddSelfCancel_MultiInput() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubAddSelfCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, Rewrite(engine, a + b + c - b));

    BF_TEST(Op(r) == OpType::Add);
    BF_TEST(InputSize(r) == 2);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == c; }));

    return 0;
}

int TestSubMulLinearCancel() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubMulLinearCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x * 5 - x));

    BF_TEST(Op(r) == OpType::Mul);
    BF_TEST(InputSize(r) == 2);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 4u); }));

    return 0;
}

int TestSubMulLinearCancel_ToBase() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubMulLinearCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x * 2 - x));

    BF_TEST(r == x);

    return 0;
}

int TestSubMulLinearCancel_ToZero() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubMulLinearCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x * 1 - x));

    BF_TEST(Op(r) == OpType::Const);
    BF_TEST(EqualChunkValue(r, 0u));

    return 0;
}

int TestSubMulLinearCancel_WithExtraFactors() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubMulLinearCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r, Rewrite(engine, x * y * 3 - x));

    BF_TEST(Op(r) == OpType::Mul);
    BF_TEST(InputSize(r) == 3);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == y; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 2u); }));

    return 0;
}

int TestMulDivConstantReduction() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_MulDivConstantReduction_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x * 12 / 3));

    BF_TEST(Op(r) == OpType::Mul);
    BF_TEST(InputSize(r) == 2);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 4u); }));

    return 0;
}

int TestMulDivConstantReduction_ToBase() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_MulDivConstantReduction_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x * 8 / 8));

    BF_TEST(r == x);

    return 0;
}

int TestMulDivConstantReduction_WithExtraFactors() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_MulDivConstantReduction_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r, Rewrite(engine, x * y * 18 / 6));

    BF_TEST(Op(r) == OpType::Mul);
    BF_TEST(InputSize(r) == 3);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == y; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 3u); }));

    return 0;
}

int TestMulToPow() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_MulToPow_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_CombineMulPow_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x * x));

    BF_TEST(Op(r) == OpType::Pow);
    BF_TEST(InputSize(r) == 2);

    BF_TEST(Input(r, 0) == x);
    BF_TEST(EqualChunkValue(Input(r, 1), 2u));

    return 0;
}

int TestMulToPow_TripleMultiplicity() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_MulToPow_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_CombineMulPow_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x * x * x));

    BF_TEST(Op(r) == OpType::Pow);
    BF_TEST(InputSize(r) == 2);

    BF_TEST(Input(r, 0) == x);
    BF_TEST(EqualChunkValue(Input(r, 1), 3u));

    return 0;
}

int TestMulToPow_MixedFactors() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_MulToPow_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_CombineMulPow_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r, Rewrite(engine, x * x * y));

    BF_TEST(Op(r) == OpType::Mul);
    BF_TEST(InputSize(r) == 2);

    bool foundPow = false;
    bool foundY = false;

    for (BitFlow::Core::Ids::ExprId inId : ExprOf(r).inputs) {
        ExprRef in(r.store, inId);

        if (Op(in) == OpType::Pow) {
            foundPow = true;

            BF_TEST(Input(in, 0) == x);
            BF_TEST(EqualChunkValue(Input(in, 1), 2u));
        }

        if (in == y)
            foundY = true;
    }

    BF_TEST(foundPow);
    BF_TEST(foundY);

    return 0;
}

int TestMulToPow_MultipleGroups() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_MulToPow_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_CombineMulPow_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r, Rewrite(engine, x * x * y * y));

    BF_TEST(Op(r) == OpType::Mul);
    BF_TEST(InputSize(r) == 2);

    int powCount = 0;
    bool foundX = false;
    bool foundY = false;

    for (BitFlow::Core::Ids::ExprId inId : ExprOf(r).inputs) {
        ExprRef in(r.store, inId);

        BF_TEST(Op(in) == OpType::Pow);

        if (Input(in, 0) == x) {
            foundX = true;
            BF_TEST(EqualChunkValue(Input(in, 1), 2u));
        }

        if (Input(in, 0) == y) {
            foundY = true;
            BF_TEST(EqualChunkValue(Input(in, 1), 2u));
        }

        ++powCount;
    }

    BF_TEST(foundX);
    BF_TEST(foundY);
    BF_TEST(powCount == 2);

    return 0;
}

int TestCombineMulPow_BaseAndPow() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_CombineMulPow_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x * x.Pow(2)));

    BF_TEST(Op(r) == OpType::Pow);
    BF_TEST(InputSize(r) == 2);

    BF_TEST(Input(r, 0) == x);
    BF_TEST(EqualChunkValue(Input(r, 1), 3u));

    return 0;
}

int TestCombineMulPow_PowAndBase() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_CombineMulPow_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x.Pow(4) * x));

    BF_TEST(Op(r) == OpType::Pow);
    BF_TEST(InputSize(r) == 2);

    BF_TEST(Input(r, 0) == x);
    BF_TEST(EqualChunkValue(Input(r, 1), 5u));

    return 0;
}

int TestCombineMulPow_TwoPows() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_CombineMulPow_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x.Pow(2) * x.Pow(5)));

    BF_TEST(Op(r) == OpType::Pow);
    BF_TEST(InputSize(r) == 2);

    BF_TEST(Input(r, 0) == x);
    BF_TEST(EqualChunkValue(Input(r, 1), 7u));

    return 0;
}

int TestCombineMulPow_MixedFactors() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_CombineMulPow_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r, Rewrite(engine, x * x.Pow(2) * y));

    BF_TEST(Op(r) == OpType::Mul);
    BF_TEST(InputSize(r) == 2);

    bool foundPow = false;
    bool foundY = false;

    for (BitFlow::Core::Ids::ExprId inId : ExprOf(r).inputs) {
        ExprRef in(r.store, inId);

        if (Op(in) == OpType::Pow) {
            foundPow = true;

            BF_TEST(Input(in, 0) == x);
            BF_TEST(EqualChunkValue(Input(in, 1), 3u));
        }

        if (in == y)
            foundY = true;
    }

    BF_TEST(foundPow);
    BF_TEST(foundY);

    return 0;
}

int main() {
    BF_RUN_TEST(TestAddFold);
    BF_RUN_TEST(TestSubConstFold);
    BF_RUN_TEST(TestSubConstFold_MultiConst);
    BF_RUN_TEST(TestSubConstFold_Cancel);

    BF_RUN_TEST(TestSubAddSelfCancel);
    BF_RUN_TEST(TestSubAddSelfCancel_MultiInput);

    BF_RUN_TEST(TestSubMulLinearCancel);
    BF_RUN_TEST(TestSubMulLinearCancel_ToBase);
    BF_RUN_TEST(TestSubMulLinearCancel_ToZero);
    BF_RUN_TEST(TestSubMulLinearCancel_WithExtraFactors);

    BF_RUN_TEST(TestMulDivConstantReduction);
    BF_RUN_TEST(TestMulDivConstantReduction_ToBase);
    BF_RUN_TEST(TestMulDivConstantReduction_WithExtraFactors);

    BF_RUN_TEST(TestMulToPow);
    BF_RUN_TEST(TestMulToPow_TripleMultiplicity);
    BF_RUN_TEST(TestMulToPow_MixedFactors);
    BF_RUN_TEST(TestMulToPow_MultipleGroups);

    BF_RUN_TEST(TestCombineMulPow_BaseAndPow);
    BF_RUN_TEST(TestCombineMulPow_PowAndBase);
    BF_RUN_TEST(TestCombineMulPow_TwoPows);
    BF_RUN_TEST(TestCombineMulPow_MixedFactors);

    return 0;
}