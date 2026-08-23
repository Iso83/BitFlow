#include "TestAssert.h"
#include "common/Expr.h"
#include "common/Rule.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestXorOrdering() {
    MakeExprStore(32);
    const auto rule = Normalize::Get_Order_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto y = V("y");
    auto x = V("x");

    BF_SAFE_REWRITE(r, BF_REWRITE(y ^ x ^ y ^ x));

    CPPTEST_ASSERT(Op(r) == OpType::Xor);
    CPPTEST_ASSERT(InputSize(r) == 4);
    CPPTEST_ASSERT(Input(r, 0) == x);
    CPPTEST_ASSERT(Input(r, 1) == x);
    CPPTEST_ASSERT(Input(r, 2) == y);
    CPPTEST_ASSERT(Input(r, 3) == y);

    return 0;
}

int main() {
    CPPTEST_RUN(TestXorOrdering);
    return 0;
}
