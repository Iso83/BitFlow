#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

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

    CPPTEST_ASSERT(Op(r) == OpType::And);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef inA) { return inA == a; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
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

    CPPTEST_ASSERT(Op(r) == OpType::And);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef inA) { return inA == a; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
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

    CPPTEST_ASSERT(Op(r) == OpType::And);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef inA) { return inA == a; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
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

    CPPTEST_ASSERT(Op(r) == OpType::Xor);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef inD) { return inD == d; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
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

    CPPTEST_ASSERT(r == expr);
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

    CPPTEST_ASSERT(Op(r) == OpType::Xor);
    CPPTEST_ASSERT(InputSize(r) == 3);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef inB) { return inB == b; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef inE) { return inE == e; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
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

    CPPTEST_ASSERT(Op(r) == OpType::Xor);
    CPPTEST_ASSERT(InputSize(r) == 2);

    // untouched: (b & c)
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inB) { return inB == b; }) &&
               AnyInput(in, [&](ExprRef inC) { return inC == c; });
    }));

    // factored: a & (b ^ c)
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
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

    CPPTEST_ASSERT(r == expr);
    return 0;
}

int TestDistributeAndOverOr_Basic() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_DistributeAndOverOr_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE((a & b) | (a & c)));

    CPPTEST_ASSERT(Op(r) == OpType::And);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef inA) { return inA == a; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::Or)
            return false;

        return AnyInput(in, [&](ExprRef inB) { return inB == b; }) &&
               AnyInput(in, [&](ExprRef inC) { return inC == c; });
    }));

    return 0;
}

int TestDistributeAndOverOr_WithUntouchedTerm() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_DistributeAndOverOr_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");

    BF_SAFE_REWRITE(r, BF_REWRITE((a & b) | (a & c) | d));

    CPPTEST_ASSERT(Op(r) == OpType::Or);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef inD) { return inD == d; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) && AnyInput(in, [&](ExprRef inner) {
                   if (Op(inner) != OpType::Or)
                       return false;

                   return AnyInput(inner, [&](ExprRef inB) { return inB == b; }) &&
                          AnyInput(inner, [&](ExprRef inC) { return inC == c; });
               });
    }));

    return 0;
}

int TestDistributeAndOverOr_NoMatch() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_DistributeAndOverOr_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");
    auto expr = (a & b) | (c & d);

    BF_SAFE_REWRITE(r, BF_REWRITE(expr));

    CPPTEST_ASSERT(r == expr);
    return 0;
}

int TestDistributeOrOverAnd_Basic() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_DistributeOrOverAnd_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE((a | b) & (a | c)));

    CPPTEST_ASSERT(Op(r) == OpType::Or);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef inA) { return inA == a; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inB) { return inB == b; }) &&
               AnyInput(in, [&](ExprRef inC) { return inC == c; });
    }));

    return 0;
}

int TestDistributeOrOverAnd_WithUntouchedTerm() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_DistributeOrOverAnd_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");

    BF_SAFE_REWRITE(r, BF_REWRITE((a | b) & (a | c) & d));

    CPPTEST_ASSERT(Op(r) == OpType::And);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef inD) { return inD == d; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::Or)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) && AnyInput(in, [&](ExprRef inner) {
                   if (Op(inner) != OpType::And)
                       return false;

                   return AnyInput(inner, [&](ExprRef inB) { return inB == b; }) &&
                          AnyInput(inner, [&](ExprRef inC) { return inC == c; });
               });
    }));

    return 0;
}

int TestDistributeOrOverAnd_NoMatch() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_DistributeOrOverAnd_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");
    auto expr = (a | b) & (c | d);

    BF_SAFE_REWRITE(r, BF_REWRITE(expr));

    CPPTEST_ASSERT(r == expr);
    return 0;
}

int main() {
    CPPTEST_RUN(TestXorAndCommonFactor);
    CPPTEST_RUN(TestXorAndCommonFactor_MultiInput);
    CPPTEST_RUN(TestXorAndFactor_Basic);
    CPPTEST_RUN(TestXorAndFactor_WithUntouchedTerm);
    CPPTEST_RUN(TestXorAndFactor_NoMatch);
    CPPTEST_RUN(TestXorAndFactor_MultiFactorChoice_PicksMostFrequent);
    CPPTEST_RUN(TestXorAndFactor_MultiFactorChoice_TieBreakOnLowerId);
    CPPTEST_RUN(TestXorAndFactor_ExplosionGuard_NoGrowthRewrite);
    CPPTEST_RUN(TestDistributeAndOverOr_Basic);
    CPPTEST_RUN(TestDistributeAndOverOr_WithUntouchedTerm);
    CPPTEST_RUN(TestDistributeAndOverOr_NoMatch);
    CPPTEST_RUN(TestDistributeOrOverAnd_Basic);
    CPPTEST_RUN(TestDistributeOrOverAnd_WithUntouchedTerm);
    CPPTEST_RUN(TestDistributeOrOverAnd_NoMatch);
    return 0;
}
