#include "TestAssert.h"
#include "common/Expr.h"
#include "common/Rule.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestXorFlatten() {
    MakeExprStore(32);
    const auto rule = Normalize::Get_Flatten_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");
    auto z = V("z");

    BF_SAFE_REWRITE(r, BF_REWRITE((x ^ y) ^ z));

    CPPTEST_ASSERT(Op(r) == OpType::Xor);
    CPPTEST_ASSERT(InputSize(r) == 3);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == x; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == y; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == z; }));

    return 0;
}

int TestNotNotDoesNotFlatten() {
    MakeExprStore(32);
    const auto rule = Normalize::Get_Flatten_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, BF_REWRITE(-(-x)));

    CPPTEST_ASSERT(Op(r) == OpType::Neg);
    CPPTEST_ASSERT(InputSize(r) == 1);

    auto inner = Input(r, 0);

    CPPTEST_ASSERT(Op(inner) == OpType::Neg);
    CPPTEST_ASSERT(InputSize(inner) == 1);
    CPPTEST_ASSERT(Input(inner, 0) == x);

    return 0;
}

int main() {
    CPPTEST_RUN(TestXorFlatten);
    CPPTEST_RUN(TestNotNotDoesNotFlatten);
    return 0;
}
