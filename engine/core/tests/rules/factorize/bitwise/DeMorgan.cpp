#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

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

    CPPTEST_ASSERT(Op(r) == OpType::Not);
    CPPTEST_ASSERT(InputSize(r) == 1);
    CPPTEST_ASSERT(Op(Input(r, 0)) == OpType::And);
    CPPTEST_ASSERT(InputSize(Input(r, 0)) == 2);
    CPPTEST_ASSERT(AnyInput(Input(r, 0), [&](ExprRef inA) { return inA == a; }));
    CPPTEST_ASSERT(AnyInput(Input(r, 0), [&](ExprRef inB) { return inB == b; }));
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

    CPPTEST_ASSERT(Op(r) == OpType::Not);
    CPPTEST_ASSERT(Op(Input(r, 0)) == OpType::And);
    CPPTEST_ASSERT(InputSize(Input(r, 0)) == 3);
    CPPTEST_ASSERT(AnyInput(Input(r, 0), [&](ExprRef inA) { return inA == a; }));
    CPPTEST_ASSERT(AnyInput(Input(r, 0), [&](ExprRef inB) { return inB == b; }));
    CPPTEST_ASSERT(AnyInput(Input(r, 0), [&](ExprRef inC) { return inC == c; }));
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

    CPPTEST_ASSERT(r == expr);
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

    CPPTEST_ASSERT(Op(r) == OpType::Not);
    CPPTEST_ASSERT(InputSize(r) == 1);
    CPPTEST_ASSERT(Op(Input(r, 0)) == OpType::Or);
    CPPTEST_ASSERT(InputSize(Input(r, 0)) == 2);
    CPPTEST_ASSERT(AnyInput(Input(r, 0), [&](ExprRef inA) { return inA == a; }));
    CPPTEST_ASSERT(AnyInput(Input(r, 0), [&](ExprRef inB) { return inB == b; }));
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

    CPPTEST_ASSERT(r == expr);
    return 0;
}

int main() {
    CPPTEST_RUN(TestDeMorganAnd_Basic);
    CPPTEST_RUN(TestDeMorganAnd_FlattenedOperands);
    CPPTEST_RUN(TestDeMorganAnd_NoMatchWhenAnyOperandIsNotNegated);
    CPPTEST_RUN(TestDeMorganOr_Basic);
    CPPTEST_RUN(TestDeMorganOr_NoMatchWhenAnyOperandIsNotNegated);
    return 0;
}
