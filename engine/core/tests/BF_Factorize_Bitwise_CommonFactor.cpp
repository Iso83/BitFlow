#include <ExprTestUtils.h>
#include <RuleTestUtils.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestXorAndCommonFactor() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_XorAnd_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorAndReduction_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE((a & b) ^ (a & c)));

    BF_TEST(Op(r) == OpType::And);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef inA) { return inA == a; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::Xor)
            return false;

        return AnyInput(in, [&](ExprRef inB) { return inB == b; }) &&
               AnyInput(in, [&](ExprRef inC) { return inC == c; });
    }));

    return 0;
}

int TestXorAndCommonFactor_MultiInput() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_XorAnd_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorAndReduction_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");

    BF_SAFE_REWRITE(r, BF_REWRITE((a & b) ^ (a & c) ^ (a & d)));

    BF_TEST(Op(r) == OpType::And);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef inA) { return inA == a; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::Xor)
            return false;

        return AnyInput(in, [&](ExprRef inB) { return inB == b; }) &&
               AnyInput(in, [&](ExprRef inC) { return inC == c; }) &&
               AnyInput(in, [&](ExprRef inD) { return inD == d; });
    }));

    return 0;
}

int TestXorAndFactor_Basic() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_XorAnd_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorAndReduction_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE((a & b) ^ (a & c)));

    BF_TEST(Op(r) == OpType::And);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef inA) { return inA == a; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::Xor)
            return false;

        return AnyInput(in, [&](ExprRef inB) { return inB == b; }) &&
               AnyInput(in, [&](ExprRef inC) { return inC == c; });
    }));

    return 0;
}

int TestXorAndFactor_WithUntouchedTerm() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_XorAnd_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorAndReduction_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");

    BF_SAFE_REWRITE(r, BF_REWRITE((a & b) ^ (a & c) ^ d));

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef inD) { return inD == d; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) && AnyInput(in, [&](ExprRef inner) {
                   if (Op(inner) != OpType::Xor)
                       return false;

                   return AnyInput(inner, [&](ExprRef inB) { return inB == b; }) &&
                          AnyInput(inner, [&](ExprRef inC) { return inC == c; });
               });
    }));

    return 0;
}

int TestXorAndFactor_NoMatch() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_XorAnd_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorAndReduction_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");
    auto expr = (a & b) ^ (c & d);

    BF_SAFE_REWRITE(r, BF_REWRITE(expr));

    BF_TEST(r == expr);
    return 0;
}

int TestXorAndFactor_MultiFactorChoice_PicksMostFrequent() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_XorAnd_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorAndReduction_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");
    auto e = V("e");

    BF_SAFE_REWRITE(r, BF_REWRITE((a & b) ^ (a & c) ^ (a & d) ^ (b ^ e)));

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(InputSize(r) == 3);
    BF_TEST(AnyInput(r, [&](ExprRef inB) { return inB == b; }));
    BF_TEST(AnyInput(r, [&](ExprRef inE) { return inE == e; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) && AnyInput(in, [&](ExprRef inner) {
                   if (Op(inner) != OpType::Xor)
                       return false;

                   return InputSize(inner) == 3 && AnyInput(inner, [&](ExprRef inB) { return inB == b; }) &&
                          AnyInput(inner, [&](ExprRef inC) { return inC == c; }) &&
                          AnyInput(inner, [&](ExprRef inD) { return inD == d; });
               });
    }));

    return 0;
}

int TestXorAndFactor_MultiFactorChoice_TieBreakOnLowerId() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_XorAnd_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorAndReduction_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE((a & b) ^ (a & c) ^ (b & c)));

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(InputSize(r) == 2);

    // untouched: (b & c)
    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inB) { return inB == b; }) &&
               AnyInput(in, [&](ExprRef inC) { return inC == c; });
    }));

    // factored: a & (b ^ c)
    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) && AnyInput(in, [&](ExprRef inner) {
                   if (Op(inner) != OpType::Xor)
                       return false;

                   return InputSize(inner) == 2 && AnyInput(inner, [&](ExprRef inB) { return inB == b; }) &&
                          AnyInput(inner, [&](ExprRef inC) { return inC == c; });
               });
    }));

    return 0;
}

int TestXorAndFactor_ExplosionGuard_NoGrowthRewrite() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_XorAnd_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorAndReduction_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");
    auto e = V("e");
    auto f = V("f");

    auto expr = f ^ a & (b & c ^ d & e);

    BF_SAFE_REWRITE(r, BF_REWRITE(expr));

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
