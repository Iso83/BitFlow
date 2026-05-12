#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

static RuleEngine MakeEngine() {

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Factorize::Bitwise::Get_Distribute_Rule());
    return engine;
}

int TestAndOverXor() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto r = Rewrite(engine, a & (b ^ c));

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(InputSize(r) == 2);

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) &&
               AnyInput(in, [&](ExprRef inB) { return inB == b; });
    }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) &&
               AnyInput(in, [&](ExprRef inC) { return inC == c; });
    }));

    return 0;
}

int TestAndOverXor_Multi() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");

    auto r = Rewrite(engine, a & (b ^ c ^ d));

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(InputSize(r) == 3);

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) &&
               AnyInput(in, [&](ExprRef inB) { return inB == b; });
    }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) &&
               AnyInput(in, [&](ExprRef inC) { return inC == c; });
    }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) &&
               AnyInput(in, [&](ExprRef inD) { return inD == d; });
    }));

    return 0;
}

int TestAndMultipleOthers() {
    MakeExprStore(32);

    RuleEngine engine = MakeEngine();

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");
    auto d = V("d");

    auto r = Rewrite(engine, a & b & (c ^ d));

    BF_TEST(Op(r) == OpType::Xor);
    BF_TEST(InputSize(r) == 2);

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) &&
               AnyInput(in, [&](ExprRef inB) { return inB == b; }) &&
               AnyInput(in, [&](ExprRef inC) { return inC == c; });
    }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (Op(in) != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef inA) { return inA == a; }) &&
               AnyInput(in, [&](ExprRef inB) { return inB == b; }) &&
               AnyInput(in, [&](ExprRef inD) { return inD == d; });
    }));

    return 0;
}

int main() {
    BF_RUN_TEST(TestAndOverXor);
    BF_RUN_TEST(TestAndOverXor_Multi);
    BF_RUN_TEST(TestAndMultipleOthers);
    return 0;
}