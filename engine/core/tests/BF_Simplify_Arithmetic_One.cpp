#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>
#include <vector>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

static RuleEngine MakeEngine_Mult() {
    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Mul_One_Rule());
    return engine;
}

int TestMulOne_Nested() {
    auto x = MakeVar(1);
    auto one = ConstPool::Get(1);

    auto mul1 = MakeOp(3, OpType::Mul, {x, one});
    auto mul2 = MakeOp(4, OpType::Mul, {mul1, one});

    RuleEngine engine = MakeEngine_Mult();
    Expr* result = engine.ApplyUntilStable(mul2);

    BF_TEST(result->id == x->id);
    return 0;
}

int TestMulOne_AllOnesBecomeConstOne() {
    auto one = ConstPool::Get(1);
    auto mul = MakeOp(10, OpType::Mul, {one, one, one});

    RuleEngine engine = MakeEngine_Mult();
    Expr* result = engine.ApplyUntilStable(mul);

    BF_TEST(result->id == one->id);
    return 0;
}

int TestMulOne_CanonicalOrderRegression() {
    auto x = MakeVar(20);
    auto y = MakeVar(21);
    auto one = ConstPool::Get(1);
    auto mul = MakeOp(22, OpType::Mul, {y, one, x});

    RuleEngine engine = MakeEngine_Mult();
    Expr* result = engine.ApplyUntilStable(mul);

    BF_TEST(result->op == OpType::Mul);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->id == y->id);
    return 0;
}

int TestMulOne_Property_OneAtAnyPosition() {
    auto x = MakeVar(30);
    auto y = MakeVar(31);
    auto z = MakeVar(32);
    auto one = ConstPool::Get(1);

    for (int onePos = 0; onePos < 4; ++onePos) {
        std::vector<Expr*> inputs = {x, y, z, x};
        inputs[onePos] = one;
        auto mul = MakeOp(100 + static_cast<uint32_t>(onePos), OpType::Mul, {inputs[0], inputs[1], inputs[2], inputs[3]});

        RuleEngine engine = MakeEngine_Mult();
        Expr* result = engine.ApplyUntilStable(mul);

        BF_TEST(result->op == OpType::Mul);
        BF_TEST(result->inputs.size() == 3);
        for (Expr* in : result->inputs)
            BF_TEST(!(in->isConst() && in->constValue == 1));
    }

    return 0;
}

int main() {
    BF_RUN_TEST(TestMulOne_Nested);
    BF_RUN_TEST(TestMulOne_AllOnesBecomeConstOne);
    BF_RUN_TEST(TestMulOne_CanonicalOrderRegression);
    BF_RUN_TEST(TestMulOne_Property_OneAtAnyPosition);
    return 0;
}
