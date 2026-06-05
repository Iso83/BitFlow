#include <ExprTestUtils.h>
#include <RuleTestUtils.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestDeMorganAnd_Basic() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_DeMorganAnd_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE((~a) | (~b), false));

    BF_TEST(Op(r) == OpType::Not);
    BF_TEST(InputSize(r) == 1);
    BF_TEST(Op(Input(r, 0)) == OpType::And);
    BF_TEST(InputSize(Input(r, 0)) == 2);
    BF_TEST(AnyInput(Input(r, 0), [&](ExprRef inA) { return inA == a; }));
    BF_TEST(AnyInput(Input(r, 0), [&](ExprRef inB) { return inB == b; }));
    return 0;
}

int TestDeMorganAnd_FlattenedOperands() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_DeMorganAnd_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE(((~a) | (~b)) | (~c), false));

    BF_TEST(Op(r) == OpType::Not);
    BF_TEST(Op(Input(r, 0)) == OpType::And);
    BF_TEST(InputSize(Input(r, 0)) == 3);
    BF_TEST(AnyInput(Input(r, 0), [&](ExprRef inA) { return inA == a; }));
    BF_TEST(AnyInput(Input(r, 0), [&](ExprRef inB) { return inB == b; }));
    BF_TEST(AnyInput(Input(r, 0), [&](ExprRef inC) { return inC == c; }));
    return 0;
}

int TestDeMorganAnd_NoMatchWhenAnyOperandIsNotNegated() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_DeMorganAnd_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto expr = (~a) | b;

    BF_SAFE_REWRITE(r, BF_REWRITE(expr, false));

    BF_TEST(r == expr);
    return 0;
}

int TestDeMorganOr_Basic() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_DeMorganOr_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE((~a) & (~b), false));

    BF_TEST(Op(r) == OpType::Not);
    BF_TEST(InputSize(r) == 1);
    BF_TEST(Op(Input(r, 0)) == OpType::Or);
    BF_TEST(InputSize(Input(r, 0)) == 2);
    BF_TEST(AnyInput(Input(r, 0), [&](ExprRef inA) { return inA == a; }));
    BF_TEST(AnyInput(Input(r, 0), [&](ExprRef inB) { return inB == b; }));
    return 0;
}

int TestDeMorganOr_NoMatchWhenAnyOperandIsNotNegated() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_DeMorganOr_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto expr = (~a) & b;

    BF_SAFE_REWRITE(r, BF_REWRITE(expr, false));

    BF_TEST(r == expr);
    return 0;
}

int main() {
    BF_RUN_TEST(TestDeMorganAnd_Basic);
    BF_RUN_TEST(TestDeMorganAnd_FlattenedOperands);
    BF_RUN_TEST(TestDeMorganAnd_NoMatchWhenAnyOperandIsNotNegated);
    BF_RUN_TEST(TestDeMorganOr_Basic);
    BF_RUN_TEST(TestDeMorganOr_NoMatchWhenAnyOperandIsNotNegated);
    return 0;
}
