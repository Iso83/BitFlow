#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestXorAndReduction_Basic() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_And_Reduction_Rule());

    auto x = V("x");
    auto y = V("y");

    auto r = Rewrite(engine, x ^ (x & y));
    auto out = ExprOf(r);

    BF_TEST(out.op == OpType::And);
    BF_TEST(out.inputs.size() == 2);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        const auto& e = ExprOf(in);

        return e.op == OpType::Not && e.inputs.size() == 1 && ERef(e.inputs[0]) == y;
    }));

    return 0;
}

int TestXorAndReduction_MultiArgXor() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_And_Reduction_Rule());

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto r = Rewrite(engine, c ^ a ^ (a & b));
    auto out = ExprOf(r);

    BF_TEST(out.op == OpType::Xor);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == c; }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (ExprOf(in).op != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == a; }) && AnyInput(in, [&](ExprRef x) {
                   const auto& e = ExprOf(x);

                   return e.op == OpType::Not && e.inputs.size() == 1 && ERef(e.inputs[0]) == b;
               });
    }));

    return 0;
}

int TestXorAndReduction_AndWithManyFactors() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_And_Reduction_Rule());

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto r = Rewrite(engine, a ^ (a & b & c));
    auto out = ExprOf(r);

    BF_TEST(out.op == OpType::And);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        const auto& e = ExprOf(in);

        if (e.op != OpType::Not || e.inputs.size() != 1)
            return false;

        ExprRef inner = ERef(e.inputs[0]);

        if (ExprOf(inner).op != OpType::And)
            return false;

        return AnyInput(inner, [&](ExprRef x) { return x == b; }) && AnyInput(inner, [&](ExprRef x) { return x == c; });
    }));

    return 0;
}

int main() {
    BF_RUN_TEST(TestXorAndReduction_Basic);
    BF_RUN_TEST(TestXorAndReduction_MultiArgXor);
    BF_RUN_TEST(TestXorAndReduction_AndWithManyFactors);
    return 0;
}
