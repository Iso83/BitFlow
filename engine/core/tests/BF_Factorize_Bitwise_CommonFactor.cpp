#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

static RuleEngine MakeEngine() {

    RuleEngine engine;
    engine.Merge(BuildNormalize());
    engine.AddRule(Factorize::Bitwise::Get_Xor_And_Rule());
    return engine;
}

int TestXorAndCommonFactor() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();
    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto r = Rewrite(engine, (a & b) ^ (a & c));
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::And);
    BF_TEST(out.inputs.size() == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (GetExpr(in).op != OpType::Xor)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == b; }) && AnyInput(in, [&](ExprRef x) { return x == c; });
    }));

    return 0;
}

int TestXorAndCommonFactor_MultiInput() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();
    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");
    auto r = Rewrite(engine, (a & b) ^ (a & c) ^ (a & d));
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::And);
    BF_TEST(out.inputs.size() == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (GetExpr(in).op != OpType::Xor)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == b; }) && AnyInput(in, [&](ExprRef x) { return x == c; }) &&
               AnyInput(in, [&](ExprRef x) { return x == d; });
    }));
    return 0;
}

int TestXorAndFactor_Basic() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();
    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto r = Rewrite(engine, (a & b) ^ (a & c));
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::And);
    BF_TEST(out.inputs.size() == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (GetExpr(in).op != OpType::Xor)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == b; }) && AnyInput(in, [&](ExprRef x) { return x == c; });
    }));
    return 0;
}

int TestXorAndFactor_WithUntouchedTerm() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();
    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");
    auto r = Rewrite(engine, (a & b) ^ (a & c) ^ d);
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::Xor);
    BF_TEST(out.inputs.size() == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == d; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (GetExpr(in).op != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == a; }) && AnyInput(in, [&](ExprRef inner) {
                   if (GetExpr(inner).op != OpType::Xor)
                       return false;

                   return AnyInput(inner, [&](ExprRef x) { return x == b; }) &&
                          AnyInput(inner, [&](ExprRef x) { return x == c; });
               });
    }));
    return 0;
}

int TestXorAndFactor_NoMatch() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();
    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");
    auto expr = (a & b) ^ (c & d);

    BF_TEST(Rewrite(engine, expr) == expr);
    return 0;
}

int TestXorAndFactor_MultiFactorChoice_PicksMostFrequent() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");
    auto e = V("e");

    auto r = Rewrite(engine, (a & b) ^ (a & c) ^ (a & d) ^ (b ^ e));
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::Xor);
    BF_TEST(out.inputs.size() == 2);

    // untouched (b ^ e)
    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (GetExpr(in).op != OpType::Xor)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == b; }) && AnyInput(in, [&](ExprRef x) { return x == e; });
    }));

    // factored a & (b ^ c ^ d)
    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (GetExpr(in).op != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == a; }) && AnyInput(in, [&](ExprRef inner) {
                   if (GetExpr(inner).op != OpType::Xor)
                       return false;

                   return AnyInput(inner, [&](ExprRef x) { return x == b; }) &&
                          AnyInput(inner, [&](ExprRef x) { return x == c; }) &&
                          AnyInput(inner, [&](ExprRef x) { return x == d; }) && GetExpr(inner).inputs.size() == 3;
               });
    }));

    return 0;
}

int TestXorAndFactor_MultiFactorChoice_TieBreakOnLowerId() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto r = Rewrite(engine, (a & b) ^ (a & c) ^ (b & c));
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::Xor);
    BF_TEST(out.inputs.size() == 2);

    // untouched: (b & c)
    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (GetExpr(in).op != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == b; }) && AnyInput(in, [&](ExprRef x) { return x == c; });
    }));

    // factored: a & (b ^ c)
    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (GetExpr(in).op != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == a; }) && AnyInput(in, [&](ExprRef inner) {
                   if (GetExpr(inner).op != OpType::Xor)
                       return false;

                   return GetExpr(inner).inputs.size() == 2 && AnyInput(inner, [&](ExprRef x) { return x == b; }) &&
                          AnyInput(inner, [&](ExprRef x) { return x == c; });
               });
    }));

    return 0;
}

int TestXorAndFactor_ExplosionGuard_NoGrowthRewrite() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");
    auto e = V("e");
    auto f = V("f");

    auto expr = (a & b & c) ^ (a & d & e) ^ f;

    BF_TEST(Rewrite(engine, expr) == expr);

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
