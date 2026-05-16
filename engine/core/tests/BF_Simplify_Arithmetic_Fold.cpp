#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestAddFold() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_AddFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x + 10 + 20));

    BF_TEST(Op(r) == OpType::Add);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 30u); }));
    return 0;
}

int TestSubConstFold() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubConstFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, (x + 8) - 1));

    BF_TEST(Op(r) == OpType::Add);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 7u); }));
    return 0;
}

int TestSubConstFold_MultiConst() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubConstFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, (x + 8 + 2) - 1));

    BF_TEST(Op(r) == OpType::Add);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return EqualChunkValue(in, 9u); }));
    return 0;
}

int TestSubConstFold_Cancel() {
    MakeExprStore(32);
    const auto rule = Simplify::Arithmetic::Get_SubConstFold_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Arithmetic::Get_AddFold_Rule());
    engine.AddRule(rule);

    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, (x + 1) - 1));

    BF_TEST(r == x);
    return 0;
}

int main() {
    BF_RUN_TEST(TestAddFold);
    BF_RUN_TEST(TestSubConstFold);
    BF_RUN_TEST(TestSubConstFold_MultiConst);
    BF_RUN_TEST(TestSubConstFold_Cancel);
    return 0;
}