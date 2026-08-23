#include "TestAssert.h"
#include "common/Expr.h"
#include "common/Rule.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestAndXorReduction_RightXor() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_AndXorReduction_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r, BF_REWRITE(x & (x ^ y)));

    CPPTEST_ASSERT(Op(r) == OpType::And);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == x; }));
    CPPTEST_ASSERT(
        AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == y; }));

    return 0;
}

int TestAndXorReduction_MultiArgAnd() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_AndXorReduction_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE(c & (a ^ c) & (b ^ c)));

    CPPTEST_ASSERT(Op(r) == OpType::And);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == c; }));
    CPPTEST_ASSERT(
        AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == a; }));
    CPPTEST_ASSERT(
        AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == b; }));
    CPPTEST_ASSERT(!AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Xor; }));

    return 0;
}

int main() {
    CPPTEST_RUN(TestAndXorReduction_RightXor);
    CPPTEST_RUN(TestAndXorReduction_MultiArgAnd);
    return 0;
}
