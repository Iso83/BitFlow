#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

static RuleEngine MakeEngine() {

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Idempotent_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Complement_Rule());
    return engine;
}

int TestAndComplement() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();
    auto a = V("a");
    auto r = Rewrite(engine, a & ~a);

    BF_TEST(IsFalse(r));

    return 0;
}

int TestOrComplement() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();
    auto a = V("a");
    auto r = Rewrite(engine, a | ~a);

    BF_TEST(IsTrue(r));

    return 0;
}

int main() {
    BF_RUN_TEST(TestAndComplement);
    BF_RUN_TEST(TestOrComplement);
    return 0;
}