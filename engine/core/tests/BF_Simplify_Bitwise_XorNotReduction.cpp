#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestXorNotReduction_Basic() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Not_Reduction_Rule());

    auto a = V("a");
    auto b = V("b");

    auto r = Rewrite(engine, (a ^ b) & ~a);
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::And);
    BF_TEST(out.inputs.size() == 2);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == b; }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        const auto& e = GetExpr(in);

        return e.op == OpType::Not && e.inputs.size() == 1 && ERef(e.inputs[0]) == a;
    }));

    return 0;
}

int TestXorNotReduction_MultiXorArgs() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Not_Reduction_Rule());

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto r = Rewrite(engine, ~a & (a ^ b ^ c));
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::And);

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        const auto& e = GetExpr(in);

        return e.op == OpType::Not && e.inputs.size() == 1 && ERef(e.inputs[0]) == a;
    }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (GetExpr(in).op != OpType::Xor)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == b; }) && AnyInput(in, [&](ExprRef x) { return x == c; });
    }));

    return 0;
}

int TestXorNotReduction_IntegrationScenario() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_And_Xor_Reduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Xor_Not_Reduction_Rule());

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto r = Rewrite(engine, (a ^ b) & c & (a ^ c));
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::And);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == b; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == c; }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        const auto& e = GetExpr(in);

        return e.op == OpType::Not && e.inputs.size() == 1 && ERef(e.inputs[0]) == a;
    }));

    BF_TEST(!AnyInput(r, [&](ExprRef in) { return GetExpr(in).op == OpType::Xor; }));

    return 0;
}

int main() {
    BF_RUN_TEST(TestXorNotReduction_Basic);
    BF_RUN_TEST(TestXorNotReduction_MultiXorArgs);
    BF_RUN_TEST(TestXorNotReduction_IntegrationScenario);
    return 0;
}
