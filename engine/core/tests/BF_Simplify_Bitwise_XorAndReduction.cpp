#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

static bool HasInput(Expr* expr, Expr* needle) {
    for (Expr* in : expr->inputs) {
        if (in == needle)
            return true;
    }

    return false;
}

static bool HasNotOf(Expr* expr, Expr* child) {
    for (Expr* in : expr->inputs) {
        if (in->op == OpType::Not && in->inputs.size() == 1 && in->inputs[0] == child)
            return true;
    }

    return false;
}

int TestXorAndReduction_Basic() {
    auto x = MakeVar(1);
    auto y = MakeVar(2);

    auto xay = MakeOp(3, OpType::And, {x, y});
    auto expr = MakeOp(4, OpType::Xor, {x, xay});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_And_Reduction_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::And);
    BF_TEST(r->inputs.size() == 2);
    BF_TEST(HasInput(r, x));
    BF_TEST(HasNotOf(r, y));
    return 0;
}

int TestXorAndReduction_MultiArgXor() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);

    auto aab = MakeOp(4, OpType::And, {a, b});
    auto expr = MakeOp(5, OpType::Xor, {c, a, aab});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_And_Reduction_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::Xor);
    BF_TEST(HasInput(r, c));

    bool foundReduced = false;
    for (Expr* in : r->inputs) {
        if (in->op != OpType::And || in->inputs.size() != 2)
            continue;

        if (!HasInput(in, a))
            continue;

        for (Expr* andIn : in->inputs) {
            if (andIn->op == OpType::Not && andIn->inputs.size() == 1 && andIn->inputs[0] == b)
                foundReduced = true;
        }
    }

    BF_TEST(foundReduced);

    return 0;
}

int TestXorAndReduction_AndWithManyFactors() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);

    auto aandbAndc = MakeOp(4, OpType::And, {a, b, c});
    auto expr = MakeOp(5, OpType::Xor, {a, aandbAndc});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_And_Reduction_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::And);
    BF_TEST(HasInput(r, a));

    Expr* notNode = nullptr;
    for (Expr* in : r->inputs) {
        if (in->op == OpType::Not && in->inputs.size() == 1)
            notNode = in;
    }

    BF_TEST(notNode != nullptr);
    BF_TEST(notNode->inputs[0]->op == OpType::And);
    BF_TEST(HasInput(notNode->inputs[0], b));
    BF_TEST(HasInput(notNode->inputs[0], c));
    return 0;
}

int main() {
    BF_RUN_TEST(TestXorAndReduction_Basic);
    BF_RUN_TEST(TestXorAndReduction_MultiArgXor);
    BF_RUN_TEST(TestXorAndReduction_AndWithManyFactors);
    return 0;
}
