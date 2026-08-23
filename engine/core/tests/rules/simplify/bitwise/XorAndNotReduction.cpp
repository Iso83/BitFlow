#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestXorAndNotReduction_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorAndNotReduction_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE((a ^ b) & ~a));

    CPPTEST_ASSERT(Op(r) == OpType::And);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == b; }));
    CPPTEST_ASSERT(
        AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == a; }));
    return 0;
}

int TestXorAndNotReduction_MultiXorArgs() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorAndNotReduction_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE(~a & (a ^ b ^ c)));

    CPPTEST_ASSERT(Op(r) == OpType::And);
    CPPTEST_ASSERT(
        AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == a; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::Xor)
            return false;

        return AnyInput(in, [&](ExprRef inB) { return inB == b; }) &&
               AnyInput(in, [&](ExprRef inC) { return inC == c; });
    }));

    return 0;
}

int TestXorAndNotReduction_IntegrationScenario() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorAndNotReduction_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE((a ^ b) & c & (a ^ c)));

    CPPTEST_ASSERT(Op(r) == OpType::And);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == b; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == c; }));
    CPPTEST_ASSERT(
        AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == a; }));
    CPPTEST_ASSERT(!AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Xor; }));
    return 0;
}

int main() {
    CPPTEST_RUN(TestXorAndNotReduction_Basic);
    CPPTEST_RUN(TestXorAndNotReduction_MultiXorArgs);
    CPPTEST_RUN(TestXorAndNotReduction_IntegrationScenario);
    return 0;
}
