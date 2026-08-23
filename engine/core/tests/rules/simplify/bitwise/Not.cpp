#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestNotDoubleNegation() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_Not_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, BF_REWRITE(~~x));

    CPPTEST_ASSERT(r == x);
    return 0;
}

int TestNotConst() {
    MakeExprStore(4);
    const auto rule = Simplify::Bitwise::Get_Not_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    BF_SAFE_REWRITE(r, BF_REWRITE(~C(0b1010)));

    CPPTEST_ASSERT(EqualChunkValue(r, 0b0101u));
    return 0;
}

int TestNotPushdown_And() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_NotPushdown_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Not_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE(~((~a) & b)));

    CPPTEST_ASSERT(Op(r) == OpType::Or);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == a; }));
    CPPTEST_ASSERT(
        AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == b; }));
    return 0;
}

int TestNotPushdown_Or() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_NotPushdown_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Not_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE(~((~a) | b)));

    CPPTEST_ASSERT(Op(r) == OpType::And);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == a; }));
    CPPTEST_ASSERT(
        AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == b; }));
    return 0;
}

int TestNotXor() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_NotXor_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE(~(a ^ b)));

    CPPTEST_ASSERT(Op(r) == OpType::Xor);
    CPPTEST_ASSERT(InputSize(r) == 3);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == a; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == b; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Const && IsTrue(in); }));
    return 0;
}

int main() {
    CPPTEST_RUN(TestNotDoubleNegation);
    CPPTEST_RUN(TestNotConst);
    CPPTEST_RUN(TestNotPushdown_And);
    CPPTEST_RUN(TestNotPushdown_Or);
    CPPTEST_RUN(TestNotXor);
    return 0;
}
