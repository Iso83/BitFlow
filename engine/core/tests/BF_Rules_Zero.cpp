#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestAndZero() {
    auto x = MakeVar(1, OpType::Add);
    auto zero = ConstPool::Get(0);

    auto add1 = MakeOp(3, OpType::Add, {x, zero});
    auto add2 = MakeOp(4, OpType::Add, {add1, zero});

    RuleEngine engine;
    engine.AddRule(Simplify::Get_Add_Zero_Rule());

    Expr* result = engine.ApplyUntilStable(add2);

    BF_TEST(result->id == x->id);
    return 0;
}

int TestXorZero() {
    auto x = MakeVar(10, OpType::Xor);
    auto zero = ConstPool::Get(0);

    auto xor1 = MakeOp(12, OpType::Xor, {x, zero});
    auto xor2 = MakeOp(13, OpType::Xor, {zero, xor1});

    RuleEngine engine;
    engine.AddRule(Simplify::Get_Xor_Zero_Rule());

    Expr* result = engine.ApplyUntilStable(xor2);

    BF_TEST(result->id == x->id);
    return 0;
}

int main() {
    BF_RUN_TEST(TestAndZero);
    BF_RUN_TEST(TestXorZero);
    return 0;
}