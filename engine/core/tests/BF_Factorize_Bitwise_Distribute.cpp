#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestAndOverXor() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);

    auto xorNode = MakeOp(10, OpType::Xor, {b, c});
    auto expr = MakeOp(11, OpType::And, {a, xorNode});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Factorize::Bitwise::Get_Distribute_Rule());

    ExprOld* result = engine.Rewrite(expr);

    BF_TEST(result->op == OpType::Xor);
    BF_TEST(result->inputs.size() == 2);

    return 0;
}

int TestAndOverXor_Multi() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);
    auto d = MakeVar(4);

    auto xorNode = MakeOp(20, OpType::Xor, {b, c, d});
    auto expr = MakeOp(21, OpType::And, {a, xorNode});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Factorize::Bitwise::Get_Distribute_Rule());

    ExprOld* result = engine.Rewrite(expr);

    BF_TEST(result->op == OpType::Xor);
    BF_TEST(result->inputs.size() == 3);

    return 0;
}

int TestAndMultipleOthers() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);
    auto d = MakeVar(4);

    auto xorNode = MakeOp(30, OpType::Xor, {c, d});
    auto expr = MakeOp(31, OpType::And, {a, b, xorNode});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Factorize::Bitwise::Get_Distribute_Rule());

    ExprOld* result = engine.Rewrite(expr);

    BF_TEST(result->op == OpType::Xor);
    BF_TEST(result->inputs.size() == 2);

    return 0;
}

int main() {
    BF_RUN_TEST(TestAndOverXor);
    BF_RUN_TEST(TestAndOverXor_Multi);
    BF_RUN_TEST(TestAndMultipleOthers);
    return 0;
}