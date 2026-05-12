#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestRotateModuloBitwidth_ReducesConstantAmount() {
    MakeExprStore(32);
    const auto rule = Normalize::Bitwise::Get_RotateModulo_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x.RotL(35)));

    BF_TEST(Op(r) == OpType::RotL);
    BF_TEST(InputSize(r) == 2);
    BF_TEST(Input(r, 0) == x);
    BF_TEST(EqualChunkValue(Input(r, 1), 3u));

    return 0;
}

int main() {
    BF_RUN_TEST(TestRotateModuloBitwidth_ReducesConstantAmount);
    return 0;
}
