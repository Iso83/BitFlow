#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <vector>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

static RuleEngine MakeEngine_Add() {
    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Add_Zero_Rule());
    return engine;
}

static RuleEngine MakeEngine_Mult() {
    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Mul_Zero_Rule());
    return engine;
}

static RuleEngine MakeEngine_Sub() {
    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Sub_Zero_Rule());
    return engine;
}

int TestAddZero_Nested() {
    auto x = MakeVar(1);
    auto zero = ConstPool::Get(0);

    auto add1 = MakeOp(3, OpType::Add, {x, zero});
    auto add2 = MakeOp(4, OpType::Add, {add1, zero});

    RuleEngine engine = MakeEngine_Add();
    Expr* result = engine.ApplyUntilStable(add2);

    BF_TEST(result->id == x->id);
    return 0;
}

int TestAddZero_AllZerosBecomeConstZero() {
    auto zero = ConstPool::Get(0);
    auto add = MakeOp(11, OpType::Add, {zero, zero, zero});

    RuleEngine engine = MakeEngine_Add();
    Expr* result = engine.ApplyUntilStable(add);

    BF_TEST(result->id == zero->id);
    return 0;
}

int TestAddZero_CanonicalOrderRegression() {
    auto x = MakeVar(20);
    auto y = MakeVar(21);
    auto zero = ConstPool::Get(0);
    auto add = MakeOp(22, OpType::Add, {y, zero, x});

    RuleEngine engine = MakeEngine_Add();
    Expr* result = engine.ApplyUntilStable(add);

    BF_TEST(result->op == OpType::Add);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->id == y->id);
    return 0;
}

int TestAddZero_Property_ZeroAtAnyPosition() {
    auto x = MakeVar(30);
    auto y = MakeVar(31);
    auto z = MakeVar(32);
    auto zero = ConstPool::Get(0);

    for (int zeroPos = 0; zeroPos < 4; ++zeroPos) {
        std::vector<Expr*> inputs = {x, y, z, x};
        inputs[zeroPos] = zero;
        auto add =
            MakeOp(100 + static_cast<uint32_t>(zeroPos), OpType::Add, {inputs[0], inputs[1], inputs[2], inputs[3]});

        RuleEngine engine = MakeEngine_Add();
        Expr* result = engine.ApplyUntilStable(add);

        BF_TEST(result->op == OpType::Add);
        BF_TEST(result->inputs.size() == 3);
        for (Expr* in : result->inputs)
            BF_TEST(!(in->isConst() && in->constValue == 0));
    }

    return 0;
}

int TestMulZero_Nested() {
    auto x = MakeVar(200);
    auto zero = ConstPool::Get(0);

    auto mul1 = MakeOp(203, OpType::Mul, {x, zero});
    auto mul2 = MakeOp(204, OpType::Mul, {mul1, x});

    RuleEngine engine = MakeEngine_Mult();
    Expr* result = engine.ApplyUntilStable(mul2);

    BF_TEST(result->id == zero->id);
    return 0;
}

int TestMulZero_DominanceWithMixedInputs() {
    auto x = MakeVar(210);
    auto y = MakeVar(211);
    auto zero = ConstPool::Get(0);
    auto mul = MakeOp(212, OpType::Mul, {x, y, zero});

    RuleEngine engine = MakeEngine_Mult();
    Expr* result = engine.ApplyUntilStable(mul);

    BF_TEST(result->id == zero->id);
    return 0;
}

int TestMulZero_Property_ZeroAtAnyPosition() {
    auto x = MakeVar(220);
    auto y = MakeVar(221);
    auto z = MakeVar(222);
    auto zero = ConstPool::Get(0);

    for (int zeroPos = 0; zeroPos < 4; ++zeroPos) {
        std::vector<Expr*> inputs = {x, y, z, x};
        inputs[zeroPos] = zero;
        auto mul =
            MakeOp(300 + static_cast<uint32_t>(zeroPos), OpType::Mul, {inputs[0], inputs[1], inputs[2], inputs[3]});

        RuleEngine engine = MakeEngine_Mult();
        Expr* result = engine.ApplyUntilStable(mul);

        BF_TEST(result->id == zero->id);
    }

    return 0;
}

int TestSubZero_Basic() {
    auto x = MakeVar(400);
    auto zero = ConstPool::Get(0);
    auto sub = MakeOp(401, OpType::Sub, {x, zero});

    RuleEngine engine = MakeEngine_Sub();
    Expr* result = engine.ApplyUntilStable(sub);

    BF_TEST(result->id == x->id);
    return 0;
}

int TestSubZero_GuardLeftZeroStaysSub() {
    auto x = MakeVar(410);
    auto zero = ConstPool::Get(0);
    auto sub = MakeOp(411, OpType::Sub, {zero, x});

    RuleEngine engine = MakeEngine_Sub();
    Expr* result = engine.ApplyUntilStable(sub);

    BF_TEST(result->op == OpType::Sub);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id == zero->id);
    BF_TEST(result->inputs[1]->id == x->id);
    return 0;
}

int TestSubZero_Property_RightOperandZero() {
    auto zero = ConstPool::Get(0);
    std::vector<Expr*> vars = {MakeVar(420), MakeVar(421), MakeVar(422), MakeVar(423)};

    for (size_t i = 0; i < vars.size(); ++i) {
        auto sub = MakeOp(500 + static_cast<uint32_t>(i), OpType::Sub, {vars[i], zero});

        RuleEngine engine = MakeEngine_Sub();
        Expr* result = engine.ApplyUntilStable(sub);

        BF_TEST(result->id == vars[i]->id);
    }

    return 0;
}

int main() {
    BF_RUN_TEST(TestAddZero_Nested);
    BF_RUN_TEST(TestAddZero_AllZerosBecomeConstZero);
    BF_RUN_TEST(TestAddZero_CanonicalOrderRegression);
    BF_RUN_TEST(TestAddZero_Property_ZeroAtAnyPosition);
    BF_RUN_TEST(TestMulZero_Nested);
    BF_RUN_TEST(TestMulZero_DominanceWithMixedInputs);
    BF_RUN_TEST(TestMulZero_Property_ZeroAtAnyPosition);
    BF_RUN_TEST(TestSubZero_Basic);
    BF_RUN_TEST(TestSubZero_GuardLeftZeroStaysSub);
    BF_RUN_TEST(TestSubZero_Property_RightOperandZero);
    return 0;
}
