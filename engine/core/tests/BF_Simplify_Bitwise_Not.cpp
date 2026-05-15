#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestNotDoubleNegation() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_Not_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, ~~x));

    BF_TEST(r == x);
    return 0;
}

int TestNotConst() {
    MakeExprStore(4);
    const auto rule = Simplify::Bitwise::Get_Not_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    BF_SAFE_REWRITE(r, Rewrite(engine, ~C(0b1010)));

    BF_TEST(EqualChunkValue(r, 0b0101u));
    return 0;
}

int TestNotPushdown_And() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_NotPushdown_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, Rewrite(engine, ~(a & b)));

    BF_TEST(Op(r) == OpType::Or);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == a; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == b; }));
    return 0;
}

int TestNotPushdown_Or() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_NotPushdown_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, Rewrite(engine, ~(a | b)));

    BF_TEST(Op(r) == OpType::And);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == a; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == b; }));
    return 0;
}

int TestNotXor() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_NotXor_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, Rewrite(engine, ~(a ^ b)));

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(InputSize(r) == 3);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == b; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Const && IsTrue(in); }));
    return 0;
}

int main() {
    BF_RUN_TEST(TestNotDoubleNegation);
    BF_RUN_TEST(TestNotConst);
    BF_RUN_TEST(TestNotPushdown_And);
    BF_RUN_TEST(TestNotPushdown_Or);
    BF_RUN_TEST(TestNotXor);
    return 0;
}