#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
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

int TestXorNotReduction_Basic() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto na = MakeOp(3, OpType::Not, {a});
    auto axb = MakeOp(4, OpType::Xor, {a, b});
    auto expr = MakeOp(5, OpType::And, {axb, na});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Not_Reduction_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::And);
    BF_TEST(r->inputs.size() == 2);
    BF_TEST(HasInput(r, b));
    BF_TEST(HasNotOf(r, a));
    return 0;
}

int TestXorNotReduction_MultiXorArgs() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);
    auto na = MakeOp(4, OpType::Not, {a});
    auto axbxc = MakeOp(5, OpType::Xor, {a, b, c});
    auto expr = MakeOp(6, OpType::And, {na, axbxc});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Not_Reduction_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::And);
    BF_TEST(HasNotOf(r, a));

    bool hasReducedXor = false;
    for (Expr* in : r->inputs) {
        if (in->op != OpType::Xor)
            continue;

        hasReducedXor = HasInput(in, b) && HasInput(in, c);
    }

    BF_TEST(hasReducedXor);
    return 0;
}

int TestXorNotReduction_IntegrationScenario() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);

    auto axb = MakeOp(4, OpType::Xor, {a, b});
    auto axc = MakeOp(5, OpType::Xor, {a, c});
    auto expr = MakeOp(6, OpType::And, {axb, c, axc});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Not_Reduction_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::And);
    BF_TEST(HasInput(r, c));
    BF_TEST(HasInput(r, b));
    BF_TEST(HasNotOf(r, a));

    for (Expr* in : r->inputs)
        BF_TEST(in->op != OpType::Xor);

    return 0;
}

int main() {
    BF_RUN_TEST(TestXorNotReduction_Basic);
    BF_RUN_TEST(TestXorNotReduction_MultiXorArgs);
    BF_RUN_TEST(TestXorNotReduction_IntegrationScenario);
    return 0;
}
