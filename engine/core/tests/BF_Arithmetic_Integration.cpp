#include <BitFlow/core/rules/RuleEngine.h>
#include <BitFlow/core/rules/RulePipeline.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

static RuleEngine MakeArithmeticEngine() {
    RuleEngine engine;
    Add_Normalize_Rules(engine);
    Add_Simplify_Arithmetic_Rules(engine);
    Add_Factorize_Arithmetic_Rules(engine);
    return engine;
}

int TestSimplify_NegNeg() {
    auto x = MakeVar(1);
    auto expr = MakeOp(2, OpType::Neg, {MakeOp(3, OpType::Neg, {x})});

    RuleEngine engine = MakeArithmeticEngine();
    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->id == x->id);
    return 0;
}

int TestSimplify_ConstCombine_Basic() {
    RuleEngine engine = MakeArithmeticEngine();

    auto add = MakeOp(10, OpType::Add, {MakeConst(11, 5), MakeConst(12, 7)});
    auto sub = MakeOp(13, OpType::Sub, {MakeConst(14, 9), MakeConst(15, 4)});
    auto mul = MakeOp(16, OpType::Mul, {MakeConst(17, 3), MakeConst(18, 6)});
    auto div = MakeOp(19, OpType::Div, {MakeConst(20, 8), MakeConst(21, 2)});

    BF_TEST(engine.ApplyUntilStable(add)->constValue == 12u);
    BF_TEST(engine.ApplyUntilStable(sub)->constValue == 5u);
    BF_TEST(engine.ApplyUntilStable(mul)->constValue == 18u);
    BF_TEST(engine.ApplyUntilStable(div)->constValue == 4u);
    return 0;
}

int TestModZero_Guard_Preserved() {
    auto x = MakeVar(30);
    auto mod = MakeOp(31, OpType::Mod, {x, MakeConst(32, 0)});

    RuleEngine engine = MakeArithmeticEngine();
    Expr* result = engine.ApplyUntilStable(mod);

    BF_TEST(result->op == OpType::Mod);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id == x->id);
    BF_TEST(result->inputs[1]->isConst() && result->inputs[1]->constValue == 0u);
    return 0;
}

int TestFactorize_AddCommonFactor() {
    auto a = MakeVar(40);
    auto b = MakeVar(41);
    auto c = MakeVar(42);

    auto expr = MakeOp(43, OpType::Add, {MakeOp(44, OpType::Mul, {a, b}), MakeOp(45, OpType::Mul, {a, c})});

    RuleEngine engine = MakeArithmeticEngine();
    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::Mul);
    BF_TEST(result->inputs.size() == 2);
    BF_TEST(result->inputs[0]->id == a->id);
    BF_TEST(result->inputs[1]->op == OpType::Add);
    return 0;
}

int TestFactorize_Canonical_a_b_plus_b_a() {
    auto a = MakeVar(50);
    auto b = MakeVar(51);

    auto lhs = MakeOp(52, OpType::Mul, {a, b});
    auto rhs = MakeOp(53, OpType::Mul, {b, a});
    auto expr = MakeOp(54, OpType::Add, {lhs, rhs});

    RuleEngine engine = MakeArithmeticEngine();
    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::Mul);
    BF_TEST(result->inputs.size() >= 2);
    return 0;
}

int TestFactorize_AddRepeatedTermCount() {
    auto a = MakeVar(60);
    auto two = MakeConst(61, 2);

    auto term = MakeOp(62, OpType::Mul, {a, two});
    auto expr = MakeOp(63, OpType::Add, {term, term, term});

    RuleEngine engine = MakeArithmeticEngine();
    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::Mul);
    BF_TEST(result->inputs.size() == 2);
    bool hasSix = false;
    bool hasA = false;
    for (auto* in : result->inputs) {
        if (in->isConst() && in->constValue == 6u)
            hasSix = true;
        if (in->id == a->id)
            hasA = true;
    }
    BF_TEST(hasSix);
    BF_TEST(hasA);
    return 0;
}

int TestFactorize_CombineNestedMulConstants() {
    auto a = MakeVar(70);
    auto expr = MakeOp(71, OpType::Mul, {MakeConst(72, 3), MakeOp(73, OpType::Mul, {a, MakeConst(74, 2)})});

    RuleEngine engine = MakeArithmeticEngine();
    Expr* result = engine.ApplyUntilStable(expr);

    BF_TEST(result->op == OpType::Mul);
    BF_TEST(result->inputs.size() == 2);
    bool hasSix = false;
    bool hasA = false;
    for (auto* in : result->inputs) {
        if (in->isConst() && in->constValue == 6u)
            hasSix = true;
        if (in->id == a->id)
            hasA = true;
    }
    BF_TEST(hasSix);
    BF_TEST(hasA);
    return 0;
}

int TestFactorize_AddLinearMultiplicityMixedForms() {
    RuleEngine engine = MakeArithmeticEngine();
    auto a = MakeVar(80);
    auto b = MakeVar(81);

    {
        auto expr = MakeOp(82, OpType::Add, {a, a});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(result->op == OpType::Mul);
        BF_TEST(result->inputs.size() == 2);
    }

    {
        auto expr = MakeOp(83, OpType::Add, {a, a, a});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(result->op == OpType::Mul);
        bool hasThree = false;
        bool hasA = false;
        for (auto* in : result->inputs) {
            if (in->isConst() && in->constValue == 3u)
                hasThree = true;
            if (in->id == a->id)
                hasA = true;
        }
        BF_TEST(hasThree);
        BF_TEST(hasA);
    }

    {
        auto expr = MakeOp(84, OpType::Add, {a, MakeOp(85, OpType::Mul, {a, MakeConst(86, 2)})});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(result->op == OpType::Mul);
        bool hasThree = false;
        bool hasA = false;
        for (auto* in : result->inputs) {
            if (in->isConst() && in->constValue == 3u)
                hasThree = true;
            if (in->id == a->id)
                hasA = true;
        }
        BF_TEST(hasThree);
        BF_TEST(hasA);
    }

    {
        auto expr = MakeOp(87, OpType::Add, {MakeOp(88, OpType::Mul, {a, MakeConst(89, 2)}), a});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(result->op == OpType::Mul);
        bool hasThree = false;
        bool hasA = false;
        for (auto* in : result->inputs) {
            if (in->isConst() && in->constValue == 3u)
                hasThree = true;
            if (in->id == a->id)
                hasA = true;
        }
        BF_TEST(hasThree);
        BF_TEST(hasA);
    }

    {
        auto expr =
            MakeOp(90, OpType::Add, {MakeOp(91, OpType::Mul, {a, MakeConst(92, 2)}), MakeOp(93, OpType::Mul, {a, MakeConst(94, 3)})});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(result->op == OpType::Mul);
        bool hasFive = false;
        bool hasA = false;
        for (auto* in : result->inputs) {
            if (in->isConst() && in->constValue == 5u)
                hasFive = true;
            if (in->id == a->id)
                hasA = true;
        }
        BF_TEST(hasFive);
        BF_TEST(hasA);
    }

    {
        auto expr = MakeOp(95, OpType::Add, {b, a, a});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(result->op == OpType::Add);
        bool hasB = false;
        bool hasA2 = false;
        for (auto* in : result->inputs) {
            if (in->id == b->id)
                hasB = true;
            if (in->op == OpType::Mul) {
                bool hasA = false;
                bool hasTwo = false;
                for (auto* factor : in->inputs) {
                    if (factor->id == a->id)
                        hasA = true;
                    if (factor->isConst() && factor->constValue == 2u)
                        hasTwo = true;
                }
                hasA2 = hasA && hasTwo;
            }
        }
        BF_TEST(hasB);
        BF_TEST(hasA2);
    }

    {
        auto expr = MakeOp(96, OpType::Add, {MakeOp(97, OpType::Mul, {a, MakeConst(98, 0)}), a});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(result->id == a->id);
    }

    {
        auto expr = MakeOp(99,
                           OpType::Add,
                           {MakeOp(100, OpType::Mul, {a, MakeConst(101, 1)}), MakeOp(102, OpType::Mul, {a, MakeConst(103, 2)})});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(result->op == OpType::Mul);
        bool hasThree = false;
        bool hasA = false;
        for (auto* in : result->inputs) {
            if (in->isConst() && in->constValue == 3u)
                hasThree = true;
            if (in->id == a->id)
                hasA = true;
        }
        BF_TEST(hasThree);
        BF_TEST(hasA);
    }

    return 0;
}

int TestFactorize_AddLinearMultiplicity_Guards() {
    RuleEngine engine = MakeArithmeticEngine();
    auto a = MakeVar(110);
    auto b = MakeVar(111);
    auto c = MakeVar(112);

    {
        // out of linear-multiplicity scope: a + a*b
        auto expr = MakeOp(113, OpType::Add, {a, MakeOp(114, OpType::Mul, {a, b})});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(result->op == OpType::Add);
        BF_TEST(result->inputs.size() == 2);
    }

    {
        // out of linear-multiplicity scope: a<<1 + a
        auto expr = MakeOp(115, OpType::Add, {MakeOp(116, OpType::Shl, {a, MakeConst(117, 1)}), a});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(result->op == OpType::Add);
        BF_TEST(result->inputs.size() == 2);
    }

    {
        // a*(b+c) is not a linear-multiplicity term and should stay unchanged when standalone.
        auto expr = MakeOp(118, OpType::Mul, {a, MakeOp(119, OpType::Add, {b, c})});
        Expr* result = engine.ApplyUntilStable(expr);
        BF_TEST(result->op == OpType::Mul);
        BF_TEST(result->inputs.size() == 2);
    }

    return 0;
}

int main() {
    BF_RUN_TEST(TestSimplify_NegNeg);
    BF_RUN_TEST(TestSimplify_ConstCombine_Basic);
    BF_RUN_TEST(TestModZero_Guard_Preserved);
    BF_RUN_TEST(TestFactorize_AddCommonFactor);
    BF_RUN_TEST(TestFactorize_Canonical_a_b_plus_b_a);
    BF_RUN_TEST(TestFactorize_AddRepeatedTermCount);
    BF_RUN_TEST(TestFactorize_CombineNestedMulConstants);
    BF_RUN_TEST(TestFactorize_AddLinearMultiplicityMixedForms);
    BF_RUN_TEST(TestFactorize_AddLinearMultiplicity_Guards);
    return 0;
}
