#include "TestAssert.h"
#include "common/Expr.h"
#include "common/Rule.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestRotateModuloBitwidth_ReducesConstantAmount() {
    MakeExprStore(32);
    const auto rule = Normalize::Bitwise::Get_RotateModulo_Rule();

    RuleEngine engine;
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto x = V("x");

    BF_SAFE_REWRITE(r, BF_REWRITE(x.RotL(35)));

    CPPTEST_ASSERT(Op(r) == OpType::RotL);
    CPPTEST_ASSERT(InputSize(r) == 2);
    CPPTEST_ASSERT(Input(r, 0) == x);
    CPPTEST_ASSERT(EqualChunkValue(Input(r, 1), 3u));

    return 0;
}

int main() {
    CPPTEST_RUN(TestRotateModuloBitwidth_ReducesConstantAmount);
    return 0;
}
