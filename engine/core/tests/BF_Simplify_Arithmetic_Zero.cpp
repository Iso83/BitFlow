#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestAddZero_Nested() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_AddZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, (x + 0) + 0))

    BF_TEST(r == x);
    return 0;
}

int TestAddZero_AllZerosBecomeConstZero() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_AddZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    BF_SAFE_REWRITE(r, Rewrite(engine, C(0) + 0 + 0));

    BF_TEST(EqualChunkValue(r, 0u));
    return 0;
}

int TestAddZero_CanonicalOrderRegression() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_AddZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r, Rewrite(engine, y + 0 + x));

    BF_TEST(Op(r) == OpType::Add);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(Input(r, 0) == x);
    BF_TEST(Input(r, 1) == y);
    return 0;
}

int TestMulZero_Nested() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_MulZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, (x * 0) * x));

    BF_TEST(EqualChunkValue(r, 0u));
    return 0;
}

int TestMulZero_DominanceWithMixedInputs() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_MulZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r, Rewrite(engine, x * y * 0));

    BF_TEST(EqualChunkValue(r, 0u));
    return 0;
}

int TestSubZero_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x - 0));

    BF_TEST(r == x);
    return 0;
}

int TestSubZero_LeftZeroBecomesNeg() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    BF_SAFE_REWRITE(r, Rewrite(engine, C(0) - x));

    BF_TEST(Op(r) == OpType::Neg);
    BF_TEST(InputSize(r) == 1);
    BF_TEST(Input(r, 0) == x);
    return 0;
}

int TestSubSelf_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubSelf_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x - x));

    BF_TEST(EqualChunkValue(r, 0u));
    return 0;
}

int TestSubSelf_DifferentInputsStaySub() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubSelf_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    auto expr = x - y;
    BF_SAFE_REWRITE(r, Rewrite(engine, expr));

    BF_TEST(r == expr);
    return 0;
}

int TestModSelf_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_ModSelf_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x % x));

    BF_TEST(EqualChunkValue(r, 0u));
    return 0;
}

int TestModSelf_DifferentInputsStayMod() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_ModSelf_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");
    auto expr = x % y;

    BF_SAFE_REWRITE(r, Rewrite(engine, expr));

    BF_TEST(r == expr);
    return 0;
}

int TestShiftZero_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_ShiftZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    for (OpType op : {OpType::Shl, OpType::Shr}) {
        ExprRef expr = (op == OpType::Shl) ? (x << 0) : (x >> 0);
        BF_SAFE_REWRITE(r, Rewrite(engine, expr));
        BF_TEST(r == x);
    }

    return 0;
}

int TestShiftZero_GuardLeftZeroStaysShift() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_ShiftZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);
    auto x = V("x");

    for (OpType op : {OpType::Shl, OpType::Shr}) {
        ExprRef expr = (op == OpType::Shl) ? (C(0) << x) : (C(0) >> x);
        BF_SAFE_REWRITE(r, Rewrite(engine, expr));
        BF_TEST(r == expr);
    }

    return 0;
}

int TestRotateModulo_FullWidthBecomesIdentity() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_RotateZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Bitwise::Get_RotateModulo_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    for (OpType op : {OpType::RotL, OpType::RotR}) {
        ExprRef expr = (op == OpType::RotL) ? x.RotL(64) : x.RotR(64);
        BF_SAFE_REWRITE(r, Rewrite(engine, expr));
        BF_TEST(r == x);
    }

    return 0;
}

int TestRotateModulo_GuardNonConstAmount() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_RotateZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Bitwise::Get_RotateModulo_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto n = V("n");
    auto expr = x.RotR(n);

    BF_SAFE_REWRITE(r, Rewrite(engine, expr));
    BF_TEST(r == expr);
    return 0;
}

int TestRotateModulo_Property_ConstantAmounts() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_RotateZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Bitwise::Get_RotateModulo_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    for (uint32_t amount = 0; amount < 128; ++amount) {
        BF_SAFE_REWRITE(r, Rewrite(engine, x.RotR(amount)));

        const uint32_t reduced = amount % 32;

        if (reduced == 0) {
            BF_TEST(r == x);
        } else {
            BF_TEST(Op(r) == OpType::RotR);
            BF_TEST(InputSize(r) == 2);
            BF_TEST(Input(r, 0) == x);
            BF_TEST(EqualChunkValue(Input(r, 1), reduced));
        }
    }

    return 0;
}

int TestRotateModulo_CanonicalOrderRegression() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_RotateZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Bitwise::Get_RotateModulo_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r, Rewrite(engine, y + x.RotL(32)));

    BF_TEST(Op(r) == OpType::Add);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == y; }));
    return 0;
}

int main() {
    BF_RUN_TEST(TestAddZero_Nested);
    BF_RUN_TEST(TestAddZero_AllZerosBecomeConstZero);
    BF_RUN_TEST(TestAddZero_CanonicalOrderRegression);
    BF_RUN_TEST(TestMulZero_Nested);
    BF_RUN_TEST(TestMulZero_DominanceWithMixedInputs);
    BF_RUN_TEST(TestSubZero_Basic);
    BF_RUN_TEST(TestSubZero_LeftZeroBecomesNeg);
    BF_RUN_TEST(TestSubSelf_Basic);
    BF_RUN_TEST(TestSubSelf_DifferentInputsStaySub);
    BF_RUN_TEST(TestModSelf_Basic);
    BF_RUN_TEST(TestModSelf_DifferentInputsStayMod);
    BF_RUN_TEST(TestShiftZero_Basic);
    BF_RUN_TEST(TestShiftZero_GuardLeftZeroStaysShift);
    BF_RUN_TEST(TestRotateModulo_FullWidthBecomesIdentity);
    BF_RUN_TEST(TestRotateModulo_GuardNonConstAmount);
    BF_RUN_TEST(TestRotateModulo_Property_ConstantAmounts);
    BF_RUN_TEST(TestRotateModulo_CanonicalOrderRegression);
    return 0;
}
