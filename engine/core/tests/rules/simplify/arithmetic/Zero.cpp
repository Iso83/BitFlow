#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestAddZero_Nested() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_AddZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, BF_REWRITE((x + 0) + 0))

    CPPTEST_ASSERT(r == x);
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

    BF_SAFE_REWRITE(r, BF_REWRITE(C(0) + 0 + 0));

    CPPTEST_ASSERT(EqualChunkValue(r, 0u));
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

    BF_SAFE_REWRITE(r, BF_REWRITE(y + 0 + x));

    CPPTEST_ASSERT(Op(r) == OpType::Add);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(Input(r, 0) == x);
    CPPTEST_ASSERT(Input(r, 1) == y);
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

    BF_SAFE_REWRITE(r, BF_REWRITE((x * 0) * x));

    CPPTEST_ASSERT(EqualChunkValue(r, 0u));
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

    BF_SAFE_REWRITE(r, BF_REWRITE(x * y * 0));

    CPPTEST_ASSERT(EqualChunkValue(r, 0u));
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

    BF_SAFE_REWRITE(r, BF_REWRITE(x - 0));

    CPPTEST_ASSERT(r == x);
    return 0;
}

int TestPowZero_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_PowZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    BF_SAFE_REWRITE(r, BF_REWRITE(x.Pow(0)));

    CPPTEST_ASSERT(EqualChunkValue(r, 1u));
    return 0;
}

int TestPowZero_GuardExponentOneStaysPow() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_PowZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto expr = x.Pow(1);
    BF_SAFE_REWRITE(r, BF_REWRITE(expr));

    CPPTEST_ASSERT(r == expr);
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
    BF_SAFE_REWRITE(r, BF_REWRITE(C(0) - x));

    CPPTEST_ASSERT(Op(r) == OpType::Neg);
    CPPTEST_ASSERT(InputSize(r) == 1);
    CPPTEST_ASSERT(Input(r, 0) == x);
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

    BF_SAFE_REWRITE(r, BF_REWRITE(x - x));

    CPPTEST_ASSERT(EqualChunkValue(r, 0u));
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
    BF_SAFE_REWRITE(r, BF_REWRITE(expr));

    CPPTEST_ASSERT(r == expr);
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

    BF_SAFE_REWRITE(r, BF_REWRITE(x % x));

    CPPTEST_ASSERT(EqualChunkValue(r, 0u));
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

    BF_SAFE_REWRITE(r, BF_REWRITE(expr));

    CPPTEST_ASSERT(r == expr);
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
        BF_SAFE_REWRITE(r, BF_REWRITE(expr));
        CPPTEST_ASSERT(r == x);
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
        BF_SAFE_REWRITE(r, BF_REWRITE(expr));
        CPPTEST_ASSERT(r == expr);
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
        BF_SAFE_REWRITE(r, BF_REWRITE(expr));
        CPPTEST_ASSERT(r == x);
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

    BF_SAFE_REWRITE(r, BF_REWRITE(expr));
    CPPTEST_ASSERT(r == expr);
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
        BF_SAFE_REWRITE(r, BF_REWRITE(x.RotR(amount)));

        const uint32_t reduced = amount % 32;

        if (reduced == 0) {
            CPPTEST_ASSERT(r == x);
        } else {
            CPPTEST_ASSERT(Op(r) == OpType::RotR);
            CPPTEST_ASSERT(InputSize(r) == 2);
            CPPTEST_ASSERT(Input(r, 0) == x);
            CPPTEST_ASSERT(EqualChunkValue(Input(r, 1), reduced));
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

    BF_SAFE_REWRITE(r, BF_REWRITE(y + x.RotL(32)));

    CPPTEST_ASSERT(Op(r) == OpType::Add);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == x; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == y; }));
    return 0;
}

int main() {
    CPPTEST_RUN(TestAddZero_Nested);
    CPPTEST_RUN(TestAddZero_AllZerosBecomeConstZero);
    CPPTEST_RUN(TestAddZero_CanonicalOrderRegression);
    CPPTEST_RUN(TestMulZero_Nested);
    CPPTEST_RUN(TestMulZero_DominanceWithMixedInputs);
    CPPTEST_RUN(TestSubZero_Basic);
    CPPTEST_RUN(TestPowZero_Basic);
    CPPTEST_RUN(TestPowZero_GuardExponentOneStaysPow);
    CPPTEST_RUN(TestSubZero_LeftZeroBecomesNeg);
    CPPTEST_RUN(TestSubSelf_Basic);
    CPPTEST_RUN(TestSubSelf_DifferentInputsStaySub);
    CPPTEST_RUN(TestModSelf_Basic);
    CPPTEST_RUN(TestModSelf_DifferentInputsStayMod);
    CPPTEST_RUN(TestShiftZero_Basic);
    CPPTEST_RUN(TestShiftZero_GuardLeftZeroStaysShift);
    CPPTEST_RUN(TestRotateModulo_FullWidthBecomesIdentity);
    CPPTEST_RUN(TestRotateModulo_GuardNonConstAmount);
    CPPTEST_RUN(TestRotateModulo_Property_ConstantAmounts);
    CPPTEST_RUN(TestRotateModulo_CanonicalOrderRegression);
    return 0;
}
