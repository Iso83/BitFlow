#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int Test_And_ZeroDominance() {
    auto a = MakeVar(1);
    auto zero = ConstPool::Get(0);

    auto expr = MakeOp(10, OpType::And, {a, zero});

    RuleEngine eng;
    eng.AddRule(Normalize::Get_Flatten_Rule());
    eng.AddRule(Simplify::Get_And_ZeroDominance_Rule());

    Expr* res = eng.ApplyUntilStable(expr);

    BF_TEST(res->isConst && res->constValue == 0);
    return 0;
}

int Test_And_OneIdentity_Multi() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto one = ConstPool::Get(1);

    auto expr = MakeOp(10, OpType::And, {a, one, b});

    RuleEngine eng;
    eng.AddRule(Normalize::Get_Flatten_Rule());
    eng.AddRule(Simplify::Get_And_OneIdentity_Rule());

    Expr* res = eng.ApplyUntilStable(expr);

    BF_TEST(res->op == OpType::And);
    BF_TEST(res->inputs.size() == 2);
    return 0;
}

int Test_Or_OneDominance() {
    auto a = MakeVar(1);
    auto one = ConstPool::Get(1);

    auto expr = MakeOp(10, OpType::Or, {a, one});

    RuleEngine eng;
    eng.AddRule(Normalize::Get_Flatten_Rule());
    eng.AddRule(Simplify::Get_Or_OneDominance_Rule());

    Expr* res = eng.ApplyUntilStable(expr);

    BF_TEST(res->isConst && res->constValue == 1);
    return 0;
}

int Test_Or_ZeroIdentity_Multi() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto zero = ConstPool::Get(0);

    auto expr = MakeOp(10, OpType::Or, {a, zero, b});

    RuleEngine eng;
    eng.AddRule(Normalize::Get_Flatten_Rule());
    eng.AddRule(Simplify::Get_Or_ZeroIdentity_Rule());

    Expr* res = eng.ApplyUntilStable(expr);

    BF_TEST(res->op == OpType::Or);
    BF_TEST(res->inputs.size() == 2);
    return 0;
}

int main() {
    BF_RUN_TEST(Test_And_ZeroDominance);
    BF_RUN_TEST(Test_And_OneIdentity_Multi);
    BF_RUN_TEST(Test_Or_OneDominance);
    BF_RUN_TEST(Test_Or_ZeroIdentity_Multi);
    return 0;
}