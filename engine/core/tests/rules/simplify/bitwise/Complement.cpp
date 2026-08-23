#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestAndComplement() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_Complement_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(a & ~a));

    CPPTEST_ASSERT(IsFalse(r));
    return 0;
}

int TestOrComplement() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_Complement_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");

    BF_SAFE_REWRITE(r, BF_REWRITE(a | ~a));

    CPPTEST_ASSERT(IsTrue(r));
    return 0;
}

int main() {
    CPPTEST_RUN(TestAndComplement);
    CPPTEST_RUN(TestOrComplement);
    return 0;
}
