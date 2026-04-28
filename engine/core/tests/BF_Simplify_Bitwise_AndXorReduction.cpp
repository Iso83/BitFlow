#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

static bool HasInput(ExprOld* andExpr, ExprOld* needle) {
    for (ExprOld* in : andExpr->inputs) {
        if (in == needle)
            return true;
    }

    return false;
}

static bool HasNotOf(ExprOld* andExpr, ExprOld* child) {
    for (ExprOld* in : andExpr->inputs) {
        if (in->op == OpType::Not && in->inputs.size() == 1 && in->inputs[0] == child)
            return true;
    }

    return false;
}

int TestAndXorReduction_RightXor() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);
    auto xorNode = MakeOp(3, OpType::Xor, {x, y});
    auto expr = MakeOp(4, OpType::And, {x, xorNode});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());

    ExprOld* r = engine.Rewrite(expr);

    BF_TEST(r->op == OpType::And);
    BF_TEST(r->inputs.size() == 2);
    BF_TEST(HasInput(r, x));
    BF_TEST(HasNotOf(r, y));
    return 0;
}

int TestAndXorReduction_LeftXor() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);
    auto xorNode = MakeOp(3, OpType::Xor, {x, y});
    auto expr = MakeOp(4, OpType::And, {xorNode, x});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());

    ExprOld* r = engine.Rewrite(expr);

    BF_TEST(r->op == OpType::And);
    BF_TEST(r->inputs.size() == 2);
    BF_TEST(HasInput(r, x));
    BF_TEST(HasNotOf(r, y));
    return 0;
}

int TestAndXorReduction_MultiArgAnd() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);

    auto axc = MakeOp(4, OpType::Xor, {a, c});
    auto bxc = MakeOp(5, OpType::Xor, {b, c});
    auto expr = MakeOp(6, OpType::And, {c, axc, bxc});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());

    ExprOld* r = engine.Rewrite(expr);

    BF_TEST(r->op == OpType::And);
    BF_TEST(HasInput(r, c));
    BF_TEST(HasNotOf(r, a));
    BF_TEST(HasNotOf(r, b));

    for (ExprOld* in : r->inputs)
        BF_TEST(in->op != OpType::Xor);

    return 0;
}

int main() {
    BF_RUN_TEST(TestAndXorReduction_RightXor);
    BF_RUN_TEST(TestAndXorReduction_LeftXor);
    BF_RUN_TEST(TestAndXorReduction_MultiArgAnd);
    return 0;
}
