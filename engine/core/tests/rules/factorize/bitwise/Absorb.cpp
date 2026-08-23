#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestAndAbsorb() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_AndAbsorb_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto inner = a | b;

    BF_SAFE_REWRITE(r, BF_REWRITE(a & inner));

    CPPTEST_ASSERT(r == a);
    return 0;
}

int TestOrAbsorb() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_OrAbsorb_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto inner = a & b;

    BF_SAFE_REWRITE(r, BF_REWRITE(a | inner));

    CPPTEST_ASSERT(r == a);
    return 0;
}

int main() {
    CPPTEST_RUN(TestAndAbsorb);
    CPPTEST_RUN(TestOrAbsorb);
    return 0;
}
