#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestLinearMultiplicityRuleOnly() {
    Expr* a = MakeVar(1);
    Expr* expr =
        MakeOp(100, OpType::Add,
               {a, MakeOp(101, OpType::Mul, {MakeConst(102, 2), a}), MakeOp(103, OpType::Mul, {MakeConst(104, 3), a})});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Factorize::Arithmetic::Get_Add_LinearMultiplicity_Rule());

    Expr* out = engine.ApplyUntilStable(expr);
    BF_TEST(out->op == OpType::Mul);
    return 0;
}

int TestCommonFactorRuleOnly() {
    Expr* a = MakeVar(11);
    Expr* b = MakeVar(12);
    Expr* c = MakeVar(13);
    Expr* expr = MakeOp(200, OpType::Add, {MakeOp(201, OpType::Mul, {a, b}), MakeOp(202, OpType::Mul, {a, c})});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Factorize::Arithmetic::Get_Add_LinearMultiplicity_Rule());
    engine.AddRule(Factorize::Arithmetic::Get_Add_CommonFactor_Rule());

    Expr* out = engine.ApplyUntilStable(expr);
    BF_TEST(out->op == OpType::Mul);
    return 0;
}

int TestArithmeticFactorizeOrdering() {
    RuleEngine engine = BuildProfile("factorize_arithmetic_safe");

    int linearIndex = -1;
    int commonIndex = -1;
    int i = 0;
    for (const auto& stage : engine.Stages()) {
        for (const auto& rule : stage.rules) {
            if (rule.id == RuleId::Factorize_AddLinearMultiplicity)
                linearIndex = i;
            if (rule.id == RuleId::Factorize_AddCommonFactor)
                commonIndex = i;
            ++i;
        }
    }

    BF_TEST(linearIndex >= 0);
    BF_TEST(commonIndex >= 0);
    BF_TEST(linearIndex < commonIndex);
    return 0;
}

int main() {
    BF_RUN_TEST(TestLinearMultiplicityRuleOnly);
    BF_RUN_TEST(TestCommonFactorRuleOnly);
    BF_RUN_TEST(TestArithmeticFactorizeOrdering);
    return 0;
}
