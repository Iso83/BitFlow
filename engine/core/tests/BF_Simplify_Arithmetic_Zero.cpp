#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestAddZero() {
    auto x = MakeVar(1);
    auto zero = ConstPool::Get(0);

    auto add1 = MakeOp(3, OpType::Add, {x, zero});
    auto add2 = MakeOp(4, OpType::Add, {add1, zero});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Add_Zero_Rule());

    Expr* result = engine.ApplyUntilStable(add2);

    BF_TEST(result->id == x->id);
    return 0;
}

int main() {
    BF_RUN_TEST(TestAddZero);
    return 0;
}