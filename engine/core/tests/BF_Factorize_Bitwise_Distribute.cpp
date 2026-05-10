#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Core::Testing;
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
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::Xor);
    BF_TEST(out.inputs.size() == 2);

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (GetExpr(in).op != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == a; }) && AnyInput(in, [&](ExprRef x) { return x == b; });
    }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (GetExpr(in).op != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == a; }) && AnyInput(in, [&](ExprRef x) { return x == c; });
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
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::Xor);
    BF_TEST(out.inputs.size() == 3);

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (GetExpr(in).op != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == a; }) && AnyInput(in, [&](ExprRef x) { return x == b; });
    }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (GetExpr(in).op != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == a; }) && AnyInput(in, [&](ExprRef x) { return x == c; });
    }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (GetExpr(in).op != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == a; }) && AnyInput(in, [&](ExprRef x) { return x == d; });
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
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::Xor);
    BF_TEST(out.inputs.size() == 2);

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (GetExpr(in).op != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == a; }) && AnyInput(in, [&](ExprRef x) { return x == b; }) &&
               AnyInput(in, [&](ExprRef x) { return x == c; });
    }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        if (GetExpr(in).op != OpType::And)
            return false;

        return AnyInput(in, [&](ExprRef x) { return x == a; }) && AnyInput(in, [&](ExprRef x) { return x == b; }) &&
               AnyInput(in, [&](ExprRef x) { return x == d; });
    }));

    return 0;
}

int main() {
    BF_RUN_TEST(TestAndOverXor);
    BF_RUN_TEST(TestAndOverXor_Multi);
    BF_RUN_TEST(TestAndMultipleOthers);
    return 0;
}