#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestXorNotReduction_Basic() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorNotReduction_Rule());

    auto a = V("a");
    auto b = V("b");

    auto r = Rewrite(engine, (a ^ b) & ~a);

    BF_TEST(Op(r) == OpType::And);
    BF_TEST(InputSize(r) == 2);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == b; }));

    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == a; }));

    return 0;
}

int TestXorNotReduction_MultiXorArgs() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorNotReduction_Rule());

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto r = Rewrite(engine, ~a & (a ^ b ^ c));
    auto out = ExprOf(r);

    BF_TEST(out.op == OpType::And);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == a; }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::Xor)
            return false;

        return AnyInput(in, [&](ExprRef inB) { return inB == b; }) &&
               AnyInput(in, [&](ExprRef inC) { return inC == c; });
    }));

    return 0;
}

int TestXorNotReduction_IntegrationScenario() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorNotReduction_Rule());

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto r = Rewrite(engine, (a ^ b) & c & (a ^ c));
    auto out = ExprOf(r);

    BF_TEST(out.op == OpType::And);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == b; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == c; }));

    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == a; }));

    BF_TEST(!AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Xor; }));

    return 0;
}

int main() {
    BF_RUN_TEST(TestXorNotReduction_Basic);
    BF_RUN_TEST(TestXorNotReduction_MultiXorArgs);
    BF_RUN_TEST(TestXorNotReduction_IntegrationScenario);
    return 0;
}
