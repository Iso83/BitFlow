#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestAndIdempotent() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_Idempotent_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(a & a));
    CPPTEST_ASSERT(r == a);

    return 0;
}

int TestAndIdempotentMixed() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_Idempotent_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE(a & b & a));

    CPPTEST_ASSERT(Op(r) == OpType::And);
    CPPTEST_ASSERT(InputSize(r) == 2);

    CPPTEST_ASSERT(Input(r, 0) == a);
    CPPTEST_ASSERT(Input(r, 1) == b);

    return 0;
}

int TestOrIdempotent() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_Idempotent_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(a | a));

    CPPTEST_ASSERT(r == a);

    return 0;
}

int TestOrIdempotentMixed() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_Idempotent_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");

    BF_SAFE_REWRITE(r, BF_REWRITE(b | a | b));

    CPPTEST_ASSERT(Op(r) == OpType::Or);
    CPPTEST_ASSERT(InputSize(r) == 2);

    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == a; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == b; }));

    return 0;
}

int main() {
    CPPTEST_RUN(TestAndIdempotent);
    CPPTEST_RUN(TestAndIdempotentMixed);
    CPPTEST_RUN(TestOrIdempotent);
    CPPTEST_RUN(TestOrIdempotentMixed);
    return 0;
}
