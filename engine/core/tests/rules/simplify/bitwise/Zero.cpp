#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestXorZero() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorZero_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, BF_REWRITE(False() ^ (x ^ False())));

    CPPTEST_ASSERT(r == x);
    return 0;
}

int main() {
    CPPTEST_RUN(TestXorZero);
    return 0;
}
