#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

int TestXorAndCommonFactor() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);

    auto and1 = MakeOp(10, OpType::And, {a, b});
    auto and2 = MakeOp(11, OpType::And, {a, c});
    auto expr = MakeOp(12, OpType::Xor, {and1, and2});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
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

int TestXorAndCommonFactor_MultiInput() {
    auto a = MakeVar(20);
    auto b = MakeVar(21);
    auto c = MakeVar(22);
    auto d = MakeVar(23);

    auto and1 = MakeOp(30, OpType::And, {a, b});
    auto and2 = MakeOp(31, OpType::And, {a, c});
    auto and3 = MakeOp(32, OpType::And, {a, d});
    auto expr = MakeOp(33, OpType::Xor, {and1, and2, and3});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Factorize::Get_Xor_And_Rule());

    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::And);
    BF_TEST(result->inputs.size() == 2);

    Expr* common = nullptr;
    Expr* inner = nullptr;

    if (result->inputs[0]->id == a->id) {
        common = result->inputs[0];
        inner = result->inputs[1];
    } else if (result->inputs[1]->id == a->id) {
        common = result->inputs[1];
        inner = result->inputs[0];
    } else
        BF_TEST(false);

    BF_TEST(common->id == a->id);
    BF_TEST(inner->op == OpType::Xor);
    BF_TEST(inner->inputs.size() == 3);

    bool hasB = false;
    bool hasC = false;
    bool hasD = false;

    for (Expr* in : inner->inputs) {
        if (in->id == b->id)
            hasB = true;
        else if (in->id == c->id)
            hasC = true;
        else if (in->id == d->id)
            hasD = true;
    }

    BF_TEST(hasB);
    BF_TEST(hasC);
    BF_TEST(hasD);
    return 0;
}

int TestXorAndFactor_Basic() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);

    auto ab = MakeOp(10, OpType::And, {a, b});
    auto ac = MakeOp(11, OpType::And, {a, c});
    auto expr = MakeOp(12, OpType::Xor, {ab, ac});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Factorize::Get_Xor_And_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::And);
    BF_TEST(r->inputs.size() == 2);
    BF_TEST(r->inputs[0] == a);
    BF_TEST(r->inputs[1]->op == OpType::Xor);
    BF_TEST(r->inputs[1]->inputs.size() == 2);
    BF_TEST(r->inputs[1]->inputs[0] == b);
    BF_TEST(r->inputs[1]->inputs[1] == c);
    return 0;
}

int TestXorAndFactor_WithUntouchedTerm() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);
    auto d = MakeVar(4);

    auto ab = MakeOp(10, OpType::And, {a, b});
    auto ac = MakeOp(11, OpType::And, {a, c});
    auto expr = MakeOp(12, OpType::Xor, {ab, ac, d});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Factorize::Get_Xor_And_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::Xor);
    BF_TEST(r->inputs.size() == 2);

    Expr* factored = nullptr;
    Expr* untouched = nullptr;

    if (r->inputs[0]->op == OpType::And) {
        factored = r->inputs[0];
        untouched = r->inputs[1];
    } else {
        factored = r->inputs[1];
        untouched = r->inputs[0];
    }

    BF_TEST(factored != nullptr);
    BF_TEST(untouched == d);

    BF_TEST(factored->op == OpType::And);
    BF_TEST(factored->inputs.size() == 2);
    BF_TEST(factored->inputs[0] == a);
    BF_TEST(factored->inputs[1]->op == OpType::Xor);
    BF_TEST(factored->inputs[1]->inputs.size() == 2);
    BF_TEST(factored->inputs[1]->inputs[0] == b);
    BF_TEST(factored->inputs[1]->inputs[1] == c);

    return 0;
}

int TestXorAndFactor_NoMatch() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);
    auto d = MakeVar(4);

    auto ab = MakeOp(10, OpType::And, {a, b});
    auto cd = MakeOp(11, OpType::And, {c, d});
    auto expr = MakeOp(12, OpType::Xor, {ab, cd});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Factorize::Get_Xor_And_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r == expr);
    return 0;
}

int TestXorAndFactor_MultiFactorChoice_PicksMostFrequent() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);
    auto d = MakeVar(4);
    auto e = MakeVar(5);

    auto ab = MakeOp(10, OpType::And, {a, b});
    auto ac = MakeOp(11, OpType::And, {a, c});
    auto ad = MakeOp(12, OpType::And, {a, d});
    auto be = MakeOp(13, OpType::And, {b, e});
    auto expr = MakeOp(14, OpType::Xor, {ab, ac, ad, be});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Factorize::Get_Xor_And_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::Xor);
    BF_TEST(r->inputs.size() == 2);

    Expr* factored = nullptr;
    Expr* untouched = nullptr;

    for (Expr* in : r->inputs) {
        if (in->op == OpType::And && in->inputs.size() == 2 &&
            (in->inputs[0]->op == OpType::Xor || in->inputs[1]->op == OpType::Xor))
            factored = in;
        else
            untouched = in;
    }

    BF_TEST(factored != nullptr);
    BF_TEST(untouched != nullptr);
    BF_TEST(untouched->op == OpType::And);
    BF_TEST(untouched->inputs.size() == 2);
    const auto u0 = untouched->inputs[0]->id;
    const auto u1 = untouched->inputs[1]->id;
    BF_TEST((u0 == b->id && u1 == e->id) || (u0 == e->id && u1 == b->id));
    BF_TEST(factored->op == OpType::And);
    BF_TEST(factored->inputs.size() == 2);
    BF_TEST(factored->inputs[0] == a);
    BF_TEST(factored->inputs[1]->op == OpType::Xor);
    BF_TEST(factored->inputs[1]->inputs.size() == 3);
    return 0;
}

int TestXorAndFactor_MultiFactorChoice_TieBreakOnLowerId() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);

    auto ab = MakeOp(10, OpType::And, {a, b});
    auto ac = MakeOp(11, OpType::And, {a, c});
    auto bc = MakeOp(12, OpType::And, {b, c});
    auto expr = MakeOp(13, OpType::Xor, {ab, ac, bc});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Factorize::Get_Xor_And_Rule());

    Expr* r = engine.ApplyUntilStable(expr);

    BF_TEST(r->op == OpType::Xor);
    BF_TEST(r->inputs.size() == 2);

    Expr* factored = nullptr;
    Expr* untouched = nullptr;

    for (Expr* in : r->inputs) {
        if (in->op == OpType::And && in->inputs.size() == 2 &&
            (in->inputs[0]->op == OpType::Xor || in->inputs[1]->op == OpType::Xor))
            factored = in;
        else
            untouched = in;
    }

    BF_TEST(factored != nullptr);
    BF_TEST(untouched != nullptr);
    BF_TEST(untouched->op == OpType::And);
    BF_TEST(untouched->inputs.size() == 2);
    const auto t0 = untouched->inputs[0]->id;
    const auto t1 = untouched->inputs[1]->id;
    BF_TEST((t0 == b->id && t1 == c->id) || (t0 == c->id && t1 == b->id));
    BF_TEST(factored->op == OpType::And);
    BF_TEST(factored->inputs.size() == 2);
    BF_TEST(factored->inputs[0] == a);
    BF_TEST(factored->inputs[1]->op == OpType::Xor);
    BF_TEST(factored->inputs[1]->inputs.size() == 2);
    BF_TEST(factored->inputs[1]->inputs[0] == b);
    BF_TEST(factored->inputs[1]->inputs[1] == c);
    return 0;
}

int TestXorAndFactor_ExplosionGuard_NoGrowthRewrite() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);
    auto c = MakeVar(3);
    auto d = MakeVar(4);
    auto e = MakeVar(5);
    auto f = MakeVar(6);

    auto abc = MakeOp(10, OpType::And, {a, b, c});
    auto ade = MakeOp(11, OpType::And, {a, d, e});
    auto expr = MakeOp(12, OpType::Xor, {abc, ade, f});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Factorize::Get_Xor_And_Rule());

    Expr* r = engine.ApplyUntilStable(expr);
    BF_TEST(r == expr);
    return 0;
}

int main() {
    BF_RUN_TEST(TestXorAndCommonFactor);
    BF_RUN_TEST(TestXorAndCommonFactor_MultiInput);
    BF_RUN_TEST(TestXorAndFactor_Basic);
    BF_RUN_TEST(TestXorAndFactor_WithUntouchedTerm);
    BF_RUN_TEST(TestXorAndFactor_NoMatch);
    BF_RUN_TEST(TestXorAndFactor_MultiFactorChoice_PicksMostFrequent);
    BF_RUN_TEST(TestXorAndFactor_MultiFactorChoice_TieBreakOnLowerId);
    BF_RUN_TEST(TestXorAndFactor_ExplosionGuard_NoGrowthRewrite);
    return 0;
}
