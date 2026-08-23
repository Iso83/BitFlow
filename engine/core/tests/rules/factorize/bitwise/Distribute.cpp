#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Rules;

int TestAndOverXor() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_Distribute_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_SAFE_REWRITE(r, BF_REWRITE(a & (b ^ c)));

    CPPTEST_ASSERT(Op(r) == OpType::Xor);
    CPPTEST_ASSERT(InputSize(r) == 2);

    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) &&
               AnyInput(in, [&](ExprRef inB) { return inB == b; });
    }));

    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) &&
               AnyInput(in, [&](ExprRef inC) { return inC == c; });
    }));

    return 0;
}

int TestAndOverXor_Multi() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_Distribute_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");

    BF_SAFE_REWRITE(r, BF_REWRITE(a & (b ^ c ^ d)));

    CPPTEST_ASSERT(Op(r) == OpType::Xor);
    CPPTEST_ASSERT(InputSize(r) == 3);

    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) &&
               AnyInput(in, [&](ExprRef inB) { return inB == b; });
    }));

    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) &&
               AnyInput(in, [&](ExprRef inC) { return inC == c; });
    }));

    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) &&
               AnyInput(in, [&](ExprRef inD) { return inD == d; });
    }));

    return 0;
}

int TestAndMultipleOthers() {
    MakeExprStore(32);
    const auto rule = Factorize::Bitwise::Get_Distribute_Rule();

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(rule);
    BF_VALIDATE_ENGINE(engine, rule);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");

    BF_SAFE_REWRITE(r, BF_REWRITE(a & b & (c ^ d)));

    CPPTEST_ASSERT(Op(r) == OpType::Xor);
    CPPTEST_ASSERT(InputSize(r) == 2);

    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) &&
               AnyInput(in, [&](ExprRef inB) { return inB == b; }) &&
               AnyInput(in, [&](ExprRef inC) { return inC == c; });
    }));

    CPPTEST_ASSERT(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) &&
               AnyInput(in, [&](ExprRef inB) { return inB == b; }) &&
               AnyInput(in, [&](ExprRef inD) { return inD == d; });
    }));

    return 0;
}

int main() {
    CPPTEST_RUN(TestAndOverXor);
    CPPTEST_RUN(TestAndOverXor_Multi);
    CPPTEST_RUN(TestAndMultipleOthers);
    return 0;
}
