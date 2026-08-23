#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestAddFold() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_AddFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, BF_REWRITE(x + 10 + 20));

    CPPTEST_ASSERT(Op(r) == OpType::Add);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == x; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 30u); }));
    return 0;
}

int TestShiftRotateConstantFold_ShiftsUseLhsBitWidth() {
    MakeExprStore(8);
    const auto rule = Simplify::Arithmetic::Get_ShiftRotateConstantFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Bitwise::Get_RotateModulo_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto value = store.createConstant(0b0011, 4);
    auto amount = store.createConstant(1, 16);

    auto shl = store.create(OpType::Shl, {value.id, amount.id}, 16);
    BF_SAFE_REWRITE(shlResult, BF_REWRITE(shl));
    CPPTEST_ASSERT(Op(shlResult) == OpType::Const);
    CPPTEST_ASSERT(BitWidth(shlResult) == 4);
    CPPTEST_ASSERT(EqualChunkValue(shlResult, 0b0110));

    auto shr = store.create(OpType::Shr, {value.id, amount.id}, 16);
    BF_SAFE_REWRITE(shrResult, BF_REWRITE(shr));
    CPPTEST_ASSERT(Op(shrResult) == OpType::Const);
    CPPTEST_ASSERT(BitWidth(shrResult) == 4);
    CPPTEST_ASSERT(EqualChunkValue(shrResult, 0b0001));

    return 0;
}

int TestShiftRotateConstantFold_RotatesUseLhsBitWidth() {
    MakeExprStore(8);
    const auto rule = Simplify::Arithmetic::Get_ShiftRotateConstantFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Bitwise::Get_RotateModulo_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto value = store.createConstant(0b10000001, 8);
    auto amount = store.createConstant(1, 16);

    auto rotl = store.create(OpType::RotL, {value.id, amount.id}, 16);
    BF_SAFE_REWRITE(rotlResult, BF_REWRITE(rotl));
    CPPTEST_ASSERT(Op(rotlResult) == OpType::Const);
    CPPTEST_ASSERT(BitWidth(rotlResult) == 8);
    CPPTEST_ASSERT(EqualChunkValue(rotlResult, 0b00000011));

    auto rotr = store.create(OpType::RotR, {value.id, amount.id}, 16);
    BF_SAFE_REWRITE(rotrResult, BF_REWRITE(rotr));
    CPPTEST_ASSERT(Op(rotrResult) == OpType::Const);
    CPPTEST_ASSERT(BitWidth(rotrResult) == 8);
    CPPTEST_ASSERT(EqualChunkValue(rotrResult, 0b11000000));

    return 0;
}

int TestSubConstFold() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubConstFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, BF_REWRITE((x + 8) - 1));

    CPPTEST_ASSERT(Op(r) == OpType::Add);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == x; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 7u); }));
    return 0;
}

int TestSubConstFold_MultiConst() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubConstFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, BF_REWRITE((x + 8 + 2) - 1));

    CPPTEST_ASSERT(Op(r) == OpType::Add);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == x; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 9u); }));
    return 0;
}

int TestSubConstFold_Cancel() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubConstFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, BF_REWRITE((x + 1) - 1));

    CPPTEST_ASSERT(r == x);
    return 0;
}

int TestSubAddSelfCancel() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubAddSelfCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r, BF_REWRITE(x + y - x));

    CPPTEST_ASSERT(r == y);
    return 0;
}

int TestSubAddSelfCancel_MultiInput() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubAddSelfCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE(a + b + c - b));

    CPPTEST_ASSERT(Op(r) == OpType::Add);
    CPPTEST_ASSERT(InputSize(r) == 2);

    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == a; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == c; }));

    return 0;
}

int TestSubAddSelfCancel_SubRhs() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubAddSelfCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE((a + b) - (b - 2)));

    CPPTEST_ASSERT(Op(r) == OpType::Add);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == a; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 2u); }));

    return 0;
}

int TestSubAddSelfCancel_SubRhs_ToConst() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubAddSelfCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE((C(1) + b) - (b - C(2))));
    CPPTEST_ASSERT(EqualChunkValue(r, 3u));

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

    BF_SAFE_REWRITE(r, BF_REWRITE(x * 5 - x));

    CPPTEST_ASSERT(Op(r) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(r) == 2);

    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == x; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 4u); }));

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

    BF_SAFE_REWRITE(r, BF_REWRITE(x * 2 - x));

    CPPTEST_ASSERT(r == x);

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

    BF_SAFE_REWRITE(r, BF_REWRITE(x * 1 - x));

    CPPTEST_ASSERT(Op(r) == OpType::Const);
    CPPTEST_ASSERT(EqualChunkValue(r, 0u));

    return 0;
}

int TestSubMulLinearCancel_PowCoefficientCancel() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubMulLinearCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(a.Pow(5) * C(2) - C(3) * a.Pow(5)));

    CPPTEST_ASSERT(Op(r) == OpType::Neg);
    CPPTEST_ASSERT(InputSize(r) == 1);

    ExprRef powExpr = Input(r, 0);

    CPPTEST_ASSERT(Op(powExpr) == OpType::Pow);
    CPPTEST_ASSERT(InputSize(powExpr) == 2);

    ExprRef base = Input(powExpr, 0);
    ExprRef exp = Input(powExpr, 1);

    CPPTEST_ASSERT(base == a);

    CPPTEST_ASSERT(Op(exp) == OpType::Const);
    CPPTEST_ASSERT(ExprOf(exp).knownValue == 5);

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

    BF_SAFE_REWRITE(r, BF_REWRITE(x * 12 / 3));

    CPPTEST_ASSERT(Op(r) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(r) == 2);

    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == x; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 4u); }));

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

    BF_SAFE_REWRITE(r, BF_REWRITE(x * 8 / 8));

    CPPTEST_ASSERT(r == x);

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

    BF_SAFE_REWRITE(r, BF_REWRITE(x * y * 18 / 6));

    CPPTEST_ASSERT(Op(r) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(r) == 3);

    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == x; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == y; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 3u); }));

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

    BF_SAFE_REWRITE(r, BF_REWRITE(x * x));

    CPPTEST_ASSERT(Op(r) == OpType::Pow);
    CPPTEST_ASSERT(InputSize(r) == 2);

    CPPTEST_ASSERT(Input(r, 0) == x);
    CPPTEST_ASSERT(EqualChunkValue(Input(r, 1), 2u));

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

    BF_SAFE_REWRITE(r, BF_REWRITE(x * x * x));

    CPPTEST_ASSERT(Op(r) == OpType::Pow);
    CPPTEST_ASSERT(InputSize(r) == 2);

    CPPTEST_ASSERT(Input(r, 0) == x);
    CPPTEST_ASSERT(EqualChunkValue(Input(r, 1), 3u));

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

    BF_SAFE_REWRITE(r, BF_REWRITE(x * x * y));

    CPPTEST_ASSERT(Op(r) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(r) == 2);

    bool foundPow = false;
    bool foundY = false;

    for (BitFlow::Engine::Core::Ids::ExprId inId : ExprOf(r).inputs) {
        ExprRef in(r.store, inId);

        if (Op(in) == OpType::Pow) {
            foundPow = true;

            CPPTEST_ASSERT(Input(in, 0) == x);
            CPPTEST_ASSERT(EqualChunkValue(Input(in, 1), 2u));
        }

        if (in == y)
            foundY = true;
    }

    CPPTEST_ASSERT(foundPow);
    CPPTEST_ASSERT(foundY);

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

    BF_SAFE_REWRITE(r, BF_REWRITE(x * x * y * y));

    CPPTEST_ASSERT(Op(r) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(r) == 2);

    int powCount = 0;
    bool foundX = false;
    bool foundY = false;

    for (BitFlow::Engine::Core::Ids::ExprId inId : ExprOf(r).inputs) {
        ExprRef in(r.store, inId);

        CPPTEST_ASSERT(Op(in) == OpType::Pow);

        if (Input(in, 0) == x) {
            foundX = true;
            CPPTEST_ASSERT(EqualChunkValue(Input(in, 1), 2u));
        }

        if (Input(in, 0) == y) {
            foundY = true;
            CPPTEST_ASSERT(EqualChunkValue(Input(in, 1), 2u));
        }

        ++powCount;
    }

    CPPTEST_ASSERT(foundX);
    CPPTEST_ASSERT(foundY);
    CPPTEST_ASSERT(powCount == 2);

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

    BF_SAFE_REWRITE(r, BF_REWRITE(x * x.Pow(2)));

    CPPTEST_ASSERT(Op(r) == OpType::Pow);
    CPPTEST_ASSERT(InputSize(r) == 2);

    CPPTEST_ASSERT(Input(r, 0) == x);
    CPPTEST_ASSERT(EqualChunkValue(Input(r, 1), 3u));

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

    BF_SAFE_REWRITE(r, BF_REWRITE(x.Pow(4) * x));

    CPPTEST_ASSERT(Op(r) == OpType::Pow);
    CPPTEST_ASSERT(InputSize(r) == 2);

    CPPTEST_ASSERT(Input(r, 0) == x);
    CPPTEST_ASSERT(EqualChunkValue(Input(r, 1), 5u));

    return 0;
}

int TestCombineMulPow_PowAndBase_SymbolicExponent() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_CombineMulPow_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(x.Pow(a) * x));

    CPPTEST_ASSERT(Op(r) == OpType::Pow);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(Input(r, 0) == x);

    ExprRef exp = Input(r, 1);
    CPPTEST_ASSERT(Op(exp) == OpType::Add);
    CPPTEST_ASSERT(InputSize(exp) == 2);

    bool foundA = false;
    bool foundOne = false;
    for (BitFlow::Engine::Core::Ids::ExprId inId : ExprOf(exp).inputs) {
        ExprRef in(exp.store, inId);
        if (in == a)
            foundA = true;
        if (Op(in) == OpType::Const && ExprOf(in).knownValue == 1)
            foundOne = true;
    }

    CPPTEST_ASSERT(foundA);
    CPPTEST_ASSERT(foundOne);

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

    BF_SAFE_REWRITE(r, BF_REWRITE(x.Pow(2) * x.Pow(5)));

    CPPTEST_ASSERT(Op(r) == OpType::Pow);
    CPPTEST_ASSERT(InputSize(r) == 2);

    CPPTEST_ASSERT(Input(r, 0) == x);
    CPPTEST_ASSERT(EqualChunkValue(Input(r, 1), 7u));

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

    BF_SAFE_REWRITE(r, BF_REWRITE(x * x.Pow(2) * y));

    CPPTEST_ASSERT(Op(r) == OpType::Mul);
    CPPTEST_ASSERT(InputSize(r) == 2);

    bool foundPow = false;
    bool foundY = false;

    for (BitFlow::Engine::Core::Ids::ExprId inId : ExprOf(r).inputs) {
        ExprRef in(r.store, inId);

        if (Op(in) == OpType::Pow) {
            foundPow = true;

            CPPTEST_ASSERT(Input(in, 0) == x);
            CPPTEST_ASSERT(EqualChunkValue(Input(in, 1), 3u));
        }

        if (in == y)
            foundY = true;
    }

    CPPTEST_ASSERT(foundPow);
    CPPTEST_ASSERT(foundY);

    return 0;
}

int TestMulPow_NegativeExponentChain() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_CombineMulPow_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(a * -(a.Pow(4))));

    CPPTEST_ASSERT(Op(r) == OpType::Neg);
    CPPTEST_ASSERT(InputSize(r) == 1);

    ExprRef powExpr = Input(r, 0);
    CPPTEST_ASSERT(Op(powExpr) == OpType::Pow);
    CPPTEST_ASSERT(InputSize(powExpr) == 2);

    ExprRef base = Input(powExpr, 0);
    CPPTEST_ASSERT(base == a);

    ExprRef exp = Input(powExpr, 1);
    CPPTEST_ASSERT(Op(exp) == OpType::Const);
    CPPTEST_ASSERT(ExprOf(exp).knownValue == 5);

    return 0;
}

int main() {
    CPPTEST_RUN(TestAddFold);
    CPPTEST_RUN(TestSubConstFold);
    CPPTEST_RUN(TestSubConstFold_MultiConst);
    CPPTEST_RUN(TestSubConstFold_Cancel);
    CPPTEST_RUN(TestShiftRotateConstantFold_ShiftsUseLhsBitWidth);
    CPPTEST_RUN(TestShiftRotateConstantFold_RotatesUseLhsBitWidth);

    CPPTEST_RUN(TestSubAddSelfCancel);
    CPPTEST_RUN(TestSubAddSelfCancel_MultiInput);
    CPPTEST_RUN(TestSubAddSelfCancel_SubRhs);
    CPPTEST_RUN(TestSubAddSelfCancel_SubRhs_ToConst);

    CPPTEST_RUN(TestSubMulLinearCancel);
    CPPTEST_RUN(TestSubMulLinearCancel_ToBase);
    CPPTEST_RUN(TestSubMulLinearCancel_ToZero);
    CPPTEST_RUN(TestSubMulLinearCancel_PowCoefficientCancel);

    CPPTEST_RUN(TestMulDivConstantReduction);
    CPPTEST_RUN(TestMulDivConstantReduction_ToBase);
    CPPTEST_RUN(TestMulDivConstantReduction_WithExtraFactors);

    CPPTEST_RUN(TestMulToPow);
    CPPTEST_RUN(TestMulToPow_TripleMultiplicity);
    CPPTEST_RUN(TestMulToPow_MixedFactors);
    CPPTEST_RUN(TestMulToPow_MultipleGroups);

    CPPTEST_RUN(TestCombineMulPow_BaseAndPow);
    CPPTEST_RUN(TestCombineMulPow_PowAndBase);
    CPPTEST_RUN(TestCombineMulPow_PowAndBase_SymbolicExponent);
    CPPTEST_RUN(TestCombineMulPow_TwoPows);
    CPPTEST_RUN(TestCombineMulPow_MixedFactors);
    CPPTEST_RUN(TestMulPow_NegativeExponentChain);

    return 0;
}
