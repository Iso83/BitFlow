#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestAddFold() {
    auto x = MakeVar(1);
    auto c1 = MakeConst(2, 10);
    auto c2 = MakeConst(3, 20);

    auto expr = MakeOp(4, OpType::Add, {x, c1, c2});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Add_Fold_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_Add_Zero_Rule());

    ExprOld* result = engine.Rewrite(expr);

    BF_TEST(result->op == OpType::Add);
    BF_TEST(result->inputs.size() == 2);

    ExprOld* a = result->inputs[0];
    ExprOld* b = result->inputs[1];

    if (a->op == OpType::Const) {
        BF_TEST(a->constValue == 30);
        BF_TEST(b->id == x->id);
    } else {
        BF_TEST(a->id == x->id);
        BF_TEST(b->op == OpType::Const);
        BF_TEST(b->constValue == 30);
    }

    return 0;
}

int main() {
    BF_RUN_TEST(TestAddFold);
    return 0;
}