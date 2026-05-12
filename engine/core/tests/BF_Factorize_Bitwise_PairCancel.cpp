#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestXorPairCancel() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_XorPairCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorZero_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorCancel_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, Rewrite(engine, (a ^ b) ^ (a ^ c)));

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(CountExpr(r, a) == 0);
    BF_TEST(CountExpr(r, b) == 1);
    BF_TEST(CountExpr(r, c) == 1);
    return 0;
}

int TestXorPairCancel_MultiInputOddCommon() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_XorPairCancel_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorZero_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorCancel_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");
    
    BF_SAFE_REWRITE(r, Rewrite(engine, (a ^ b) ^ (a ^ c) ^ (a ^ d)));

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(InputSize(r) == 4);
    BF_TEST(CountExpr(r, a) == 1);
    BF_TEST(CountExpr(r, b) == 1);
    BF_TEST(CountExpr(r, c) == 1);
    BF_TEST(CountExpr(r, d) == 1);
    return 0;
}

int main() {
    BF_RUN_TEST(TestXorPairCancel);
    BF_RUN_TEST(TestXorPairCancel_MultiInputOddCommon);
    return 0;
}