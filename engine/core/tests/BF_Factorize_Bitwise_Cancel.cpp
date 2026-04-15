#include <BitFlow/core/ast/OpType.h>
#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestXorXorCancelPair() {
    auto a = MakeVar(40);
    auto b = MakeVar(41);
    auto c = MakeVar(42);

    auto x1 = MakeOp(50, OpType::Xor, {a, b});
    auto x2 = MakeOp(51, OpType::Xor, {a, c});
    auto expr = MakeOp(52, OpType::Xor, {x1, x2});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());
    engine.AddRule(Factorize::Bitwise::Get_Xor_Pair_Cancel_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::Xor);
    BF_TEST(result->inputs.size() == 2);

    auto i0 = result->inputs[0];
    auto i1 = result->inputs[1];

    BF_TEST((i0->id == b->id && i1->id == c->id) || (i0->id == c->id && i1->id == b->id));
    return 0;
}

int TestXorXorCancelPair_MultiInputOddCommon() {
    auto a = MakeVar(60);
    auto b = MakeVar(61);
    auto c = MakeVar(62);
    auto d = MakeVar(63);

    auto x1 = MakeOp(70, OpType::Xor, {a, b});
    auto x2 = MakeOp(71, OpType::Xor, {a, c});
    auto x3 = MakeOp(72, OpType::Xor, {a, d});
    auto expr = MakeOp(73, OpType::Xor, {x1, x2, x3});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Cancel_Rule());
    engine.AddRule(Factorize::Bitwise::Get_Xor_Pair_Cancel_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::Xor);
    BF_TEST(result->inputs.size() == 4);

    bool hasA = false;
    bool hasB = false;
    bool hasC = false;
    bool hasD = false;

    for (Expr* in : result->inputs) {
        if (in->id == a->id)
            hasA = true;
        else if (in->id == b->id)
            hasB = true;
        else if (in->id == c->id)
            hasC = true;
        else if (in->id == d->id)
            hasD = true;
    }

    BF_TEST(hasA);
    BF_TEST(hasB);
    BF_TEST(hasC);
    BF_TEST(hasD);
    return 0;
}

int main() {
    BF_RUN_TEST(TestXorXorCancelPair);
    BF_RUN_TEST(TestXorXorCancelPair_MultiInputOddCommon);
    return 0;
}