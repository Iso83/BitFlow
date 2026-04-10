#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestXorAndCommonFactor() {
    auto a = MakeVar(1, OpType::Xor);
    auto b = MakeVar(2, OpType::Xor);
    auto c = MakeVar(3, OpType::Xor);

    auto and1 = MakeOp(10, OpType::And, {a, b});
    auto and2 = MakeOp(11, OpType::And, {a, c});
    auto expr = MakeOp(12, OpType::Xor, {and1, and2});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Factorize::Get_Xor_And_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::And);
    BF_TEST(result->inputs.size() == 2);

    Expr* left = result->inputs[0];
    Expr* right = result->inputs[1];

    Expr* common = nullptr;
    Expr* inner = nullptr;

    if (left->id == a->id) {
        common = left;
        inner = right;
    } else if (right->id == a->id) {
        common = right;
        inner = left;
    } else
        BF_TEST(false);

    BF_TEST(common->id == a->id);
    BF_TEST(inner->op == OpType::Xor);
    BF_TEST(inner->inputs.size() == 2);

    auto x0 = inner->inputs[0];
    auto x1 = inner->inputs[1];

    BF_TEST((x0->id == b->id && x1->id == c->id) || (x0->id == c->id && x1->id == b->id));
    return 0;
}

int main() {
    BF_RUN_TEST(TestXorAndCommonFactor);
    return 0;
}