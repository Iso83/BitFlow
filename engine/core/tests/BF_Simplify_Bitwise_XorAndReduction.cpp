#include <ExprTestUtils.h>
#include <RuleTestUtils.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

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

    BF_TEST(Op(r) == OpType::And);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == x; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return Op(in) == OpType::Not && InputSize(in) == 1 && Input(in, 0) == y; }));
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

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == c; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) {
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

    BF_TEST(Op(r) == OpType::And);
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) {
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
    BF_RUN_TEST(TestXorAndReduction_Basic);
    BF_RUN_TEST(TestXorAndReduction_MultiArgXor);
    BF_RUN_TEST(TestXorAndReduction_AndWithManyFactors);
    return 0;
}
