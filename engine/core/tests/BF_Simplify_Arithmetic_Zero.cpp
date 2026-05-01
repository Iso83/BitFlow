#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <vector>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
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

static RuleEngine MakeEngine_ModGuard() {
    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Mod_Zero_Guard_Rule());
    return engine;
}

static RuleEngine MakeEngine_ShiftZero() {
    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Shift_Zero_Rule());
    return engine;
}

static RuleEngine MakeEngine_RotateModuloBitwidth() {
    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Rotate_Modulo_Bitwidth_Rule());
    return engine;
}

static RuleEngine MakeEngine_RotateModuloBitwidth_WithOrder() {
    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Rotate_Modulo_Bitwidth_Rule());
    return engine;
}

int TestAddZero_Nested() {
    auto x = MakeVar(1);
    auto zero = ConstPool::Get(0);

    auto add1 = MakeOp(3, OpType::Add, {x, zero});
    auto add2 = MakeOp(4, OpType::Add, {add1, zero});

    RuleEngine engine = MakeEngine_Add();
    ExprOld* result = engine.Rewrite(add2);

    BF_TEST(result->id == x->id);
    return 0;
}

int TestAddZero_AllZerosBecomeConstZero() {
    auto zero = ConstPool::Get(0);
    auto add = MakeOp(11, OpType::Add, {zero, zero, zero});

    RuleEngine engine = MakeEngine_Add();
    ExprOld* result = engine.Rewrite(add);

    BF_TEST(result->id == zero->id);
    return 0;
}

int TestAddZero_CanonicalOrderRegression() {
    auto x = MakeVar(20);
    auto y = MakeVar(21);
    auto zero = ConstPool::Get(0);
    auto add = MakeOp(22, OpType::Add, {y, zero, x});

    RuleEngine engine = MakeEngine_Add();
    ExprOld* result = engine.Rewrite(add);

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
        std::vector<ExprOld*> inputs = {x, y, z, x};
        inputs[zeroPos] = zero;
        auto add =
            MakeOp(100 + static_cast<uint32_t>(zeroPos), OpType::Add, {inputs[0], inputs[1], inputs[2], inputs[3]});

        RuleEngine engine = MakeEngine_Add();
        ExprOld* result = engine.Rewrite(add);

        BF_TEST(result->op == OpType::Add);
        BF_TEST(result->inputs.size() == 3);
        for (ExprOld* in : result->inputs)
            BF_TEST(!(in->op == OpType::Const && in->constValue == 0));
    }

    return 0;
}

int TestMulZero_Nested() {
    auto x = MakeVar(200);
    auto zero = ConstPool::Get(0);

    auto mul1 = MakeOp(203, OpType::Mul, {x, zero});
    auto mul2 = MakeOp(204, OpType::Mul, {mul1, x});

    RuleEngine engine = MakeEngine_Mult();
    ExprOld* result = engine.Rewrite(mul2);

    BF_TEST(result->id == zero->id);
    return 0;
}

int TestMulZero_DominanceWithMixedInputs() {
    auto x = MakeVar(210);
    auto y = MakeVar(211);
    auto zero = ConstPool::Get(0);
    auto mul = MakeOp(212, OpType::Mul, {x, y, zero});

    RuleEngine engine = MakeEngine_Mult();
    ExprOld* result = engine.Rewrite(mul);

    BF_TEST(result->id == zero->id);
    return 0;
}

int TestMulZero_Property_ZeroAtAnyPosition() {
    auto x = MakeVar(220);
    auto y = MakeVar(221);
    auto z = MakeVar(222);
    auto zero = ConstPool::Get(0);

    for (int zeroPos = 0; zeroPos < 4; ++zeroPos) {
        std::vector<ExprOld*> inputs = {x, y, z, x};
        inputs[zeroPos] = zero;
        auto mul =
            MakeOp(300 + static_cast<uint32_t>(zeroPos), OpType::Mul, {inputs[0], inputs[1], inputs[2], inputs[3]});

        RuleEngine engine = MakeEngine_Mult();
        ExprOld* result = engine.Rewrite(mul);

        BF_TEST(result->id == zero->id);
    }

    return 0;
}

int TestSubZero_Basic() {
    auto x = MakeVar(400);
    auto zero = ConstPool::Get(0);
    auto sub = MakeOp(401, OpType::Sub, {x, zero});

    RuleEngine engine = MakeEngine_Sub();
    ExprOld* result = engine.Rewrite(sub);

    BF_TEST(result->id == x->id);
    return 0;
}

int TestSubZero_GuardLeftZeroStaysSub() {
    auto x = MakeVar(410);
    auto zero = ConstPool::Get(0);
    auto sub = MakeOp(411, OpType::Sub, {zero, x});

    RuleEngine engine = MakeEngine_Sub();
    ExprOld* result = engine.Rewrite(sub);

    BF_TEST(result->op == OpType::Sub);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id == zero->id);
    BF_TEST(result->inputs[1]->id == x->id);
    return 0;
}

int TestSubZero_Property_RightOperandZero() {
    auto zero = ConstPool::Get(0);
    std::vector<ExprOld*> vars = {MakeVar(420), MakeVar(421), MakeVar(422), MakeVar(423)};

    for (size_t i = 0; i < vars.size(); ++i) {
        auto sub = MakeOp(500 + static_cast<uint32_t>(i), OpType::Sub, {vars[i], zero});

        RuleEngine engine = MakeEngine_Sub();
        ExprOld* result = engine.Rewrite(sub);

        BF_TEST(result->id == vars[i]->id);
    }

    return 0;
}

int TestModZero_GuardKeepsNode() {
    auto x = MakeVar(600);
    auto zero = ConstPool::Get(0);
    auto mod = MakeOp(601, OpType::Mod, {x, zero});

    RuleEngine engine = MakeEngine_ModGuard();
    ExprOld* result = engine.Rewrite(mod);

    BF_TEST(result->op == OpType::Mod);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->id == zero->id);
    return 0;
}

int TestModZero_GuardLeftZeroStaysMod() {
    auto x = MakeVar(610);
    auto zero = ConstPool::Get(0);
    auto mod = MakeOp(611, OpType::Mod, {zero, x});

    RuleEngine engine = MakeEngine_ModGuard();
    ExprOld* result = engine.Rewrite(mod);

    BF_TEST(result->op == OpType::Mod);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id == zero->id);
    BF_TEST(result->inputs[1]->id == x->id);
    return 0;
}

int TestShiftZero_Basic() {
    auto x = MakeVar(700);
    auto zero = ConstPool::Get(0);

    for (OpType op : {OpType::Shl, OpType::Shr}) {
        auto shift = MakeOp(701 + static_cast<uint32_t>(op), op, {x, zero});

        RuleEngine engine = MakeEngine_ShiftZero();
        ExprOld* result = engine.Rewrite(shift);

        BF_TEST(result->id == x->id);
    }

    return 0;
}

int TestShiftZero_GuardLeftZeroStaysShift() {
    auto x = MakeVar(710);
    auto zero = ConstPool::Get(0);

    for (OpType op : {OpType::Shl, OpType::Shr}) {
        auto shift = MakeOp(711 + static_cast<uint32_t>(op), op, {zero, x});

        RuleEngine engine = MakeEngine_ShiftZero();
        ExprOld* result = engine.Rewrite(shift);

        BF_TEST(result->op == op);
        BF_TEST(result->inputs.size() == 2);
        BF_TEST(result->inputs[0]->id == zero->id);
        BF_TEST(result->inputs[1]->id == x->id);
    }

    return 0;
}

int TestRotateModuloBitwidth_ReducesConstantAmount() {
    auto x = MakeVar(800);
    auto amount = ConstPool::Get(35);

    auto rot = MakeOp(801, OpType::RotL, {x, amount});

    RuleEngine engine = MakeEngine_RotateModuloBitwidth();
    ExprOld* result = engine.Rewrite(rot);

    BF_TEST(result->op == OpType::RotL);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->op == OpType::Const);
    BF_TEST(result->inputs[1]->constValue == 3);
    return 0;
}

int TestRotateModuloBitwidth_FullWidthBecomesIdentity() {
    auto x = MakeVar(810);
    auto amount = ConstPool::Get(64);

    for (OpType op : {OpType::RotL, OpType::RotR}) {
        auto rot = MakeOp(811 + static_cast<uint32_t>(op), op, {x, amount});

        RuleEngine engine = MakeEngine_RotateModuloBitwidth();
        ExprOld* result = engine.Rewrite(rot);

        BF_TEST(result->id == x->id);
    }

    return 0;
}

int TestRotateModuloBitwidth_GuardNonConstAmount() {
    auto x = MakeVar(820);
    auto n = MakeVar(821);
    auto rot = MakeOp(822, OpType::RotR, {x, n});

    RuleEngine engine = MakeEngine_RotateModuloBitwidth();
    ExprOld* result = engine.Rewrite(rot);

    BF_TEST(result->op == OpType::RotR);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->id == n->id);
    return 0;
}

int TestRotateModuloBitwidth_Property_ConstantAmounts() {
    auto x = MakeVar(830);

    for (uint32_t amount = 0; amount < 128; ++amount) {
        auto c = ConstPool::Get(amount);
        auto rot = MakeOp(900 + amount, OpType::RotR, {x, c});

        RuleEngine engine = MakeEngine_RotateModuloBitwidth();
        ExprOld* result = engine.Rewrite(rot);

        const uint32_t reduced = amount % 32;
        if (reduced == 0) {
            BF_TEST(result->id == x->id);
        } else {
            BF_TEST(result->op == OpType::RotR);
            BF_TEST(result->inputs.size() == 2);
            BF_TEST(result->inputs[0]->id == x->id);
            BF_TEST(result->inputs[1]->op == OpType::Const);
            BF_TEST(result->inputs[1]->constValue == reduced);
        }
    }

    return 0;
}

int TestRotateModuloBitwidth_CanonicalOrderRegression() {
    auto x = MakeVar(840);
    auto y = MakeVar(841);
    auto amount = ConstPool::Get(32);

    auto rot = MakeOp(842, OpType::RotL, {x, amount});
    auto add = MakeOp(843, OpType::Add, {y, rot});

    RuleEngine engine = MakeEngine_RotateModuloBitwidth_WithOrder();
    ExprOld* result = engine.Rewrite(add);

    BF_TEST(result->op == OpType::Add);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->id == y->id);
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
    BF_RUN_TEST(TestModZero_GuardKeepsNode);
    BF_RUN_TEST(TestModZero_GuardLeftZeroStaysMod);
    BF_RUN_TEST(TestShiftZero_Basic);
    BF_RUN_TEST(TestShiftZero_GuardLeftZeroStaysShift);
    BF_RUN_TEST(TestRotateModuloBitwidth_ReducesConstantAmount);
    BF_RUN_TEST(TestRotateModuloBitwidth_FullWidthBecomesIdentity);
    BF_RUN_TEST(TestRotateModuloBitwidth_GuardNonConstAmount);
    BF_RUN_TEST(TestRotateModuloBitwidth_Property_ConstantAmounts);
    BF_RUN_TEST(TestRotateModuloBitwidth_CanonicalOrderRegression);
    return 0;
}
