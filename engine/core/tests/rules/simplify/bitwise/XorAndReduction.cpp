#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestXorAndReduction_Basic() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorAndReduction_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");
    auto y = V("y");

    BF_SAFE_REWRITE(r, BF_REWRITE(x ^ (x & y)));

    CPPTEST_ASSERT(Op(r) == OpType::And);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == x; }));
    CPPTEST_ASSERT(
        AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == y; }));
    return 0;
}

int TestXorAndReduction_MultiArgXor() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorAndReduction_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE(c ^ a ^ (a & b)));

    CPPTEST_ASSERT(Op(r) == OpType::Xor);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == c; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) && AnyInput(in, [&](ExprRef inB) {
                   return Op(inB) == OpType::Not && InputSize(inB) == 1 && Input(inB, 0) == b;
               });
    }));
    return 0;
}

int TestXorAndReduction_AndWithManyFactors() {
    MakeExprStore(32);
    const auto rule = Simplify::Bitwise::Get_XorAndReduction_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE(a ^ (a & b & c)));

    CPPTEST_ASSERT(Op(r) == OpType::And);
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) { return in == a; }));
    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::Not || InputSize(in) != 1)
            return false;

        ExprRef inner = Input(in, 0);

        if (Op(inner) != OpType::And)
            return false;

        return AnyInput(inner, [&](ExprRef inB) { return inB == b; }) &&
               AnyInput(inner, [&](ExprRef inC) { return inC == c; });
    }));
    return 0;
}

int main() {
    CPPTEST_RUN(TestXorAndReduction_Basic);
    CPPTEST_RUN(TestXorAndReduction_MultiArgXor);
    CPPTEST_RUN(TestXorAndReduction_AndWithManyFactors);
    return 0;
}
