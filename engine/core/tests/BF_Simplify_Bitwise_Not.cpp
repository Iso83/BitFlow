#include <BitFlow/core/rules/RulePipeline.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestNotDoubleNegation() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Simplify::Bitwise::Get_Not_Rule());

    auto x = V("x");

    BF_TEST(Rewrite(engine, ~~x) == x);

    return 0;
}

int TestNotConst() {
    MakeExprStore(4);

    RuleEngine engine;
    engine.AddRule(Simplify::Bitwise::Get_Not_Rule());

    auto r = Rewrite(engine, ~C(0b1010));

    BF_TEST(EqualChunkValue(r, 0b0101u));

    return 0;
}

int TestNotPushdown_And() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Simplify::Bitwise::Get_Not_Pushdown_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Not_Rule());

    auto a = V("a");
    auto b = V("b");

    auto r = Rewrite(engine, ~(a & b));
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::Or);
    BF_TEST(out.inputs.size() == 2);

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        const auto& e = GetExpr(in);

        return e.op == OpType::Not && e.inputs.size() == 1 && ERef(e.inputs[0]) == a;
    }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        const auto& e = GetExpr(in);

        return e.op == OpType::Not && e.inputs.size() == 1 && ERef(e.inputs[0]) == b;
    }));

    return 0;
}

int TestNotPushdown_Or() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Simplify::Bitwise::Get_Not_Pushdown_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Not_Rule());

    auto a = V("a");
    auto b = V("b");

    auto r = Rewrite(engine, ~(a | b));
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::And);
    BF_TEST(out.inputs.size() == 2);

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        const auto& e = GetExpr(in);

        return e.op == OpType::Not && e.inputs.size() == 1 && ERef(e.inputs[0]) == a;
    }));

    BF_TEST(AnyInput(r, [&](ExprRef in) {
        const auto& e = GetExpr(in);

        return e.op == OpType::Not && e.inputs.size() == 1 && ERef(e.inputs[0]) == b;
    }));

    return 0;
}

int TestNotXor() {
    MakeExprStore(32);

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Not_Xor_Rule());

    auto a = V("a");
    auto b = V("b");

    auto r = Rewrite(engine, ~(a ^ b));
    auto out = GetExpr(r);

    BF_TEST(out.op == OpType::Xor);
    BF_TEST(out.inputs.size() == 3);

    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == a; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return in == b; }));
    BF_TEST(AnyInput(r, [&](ExprRef in) { return IsTrue(in); }));

    return 0;
}

int main() {
    BF_RUN_TEST(TestNotDoubleNegation);
    BF_RUN_TEST(TestNotConst);
    BF_RUN_TEST(TestNotPushdown_And);
    BF_RUN_TEST(TestNotPushdown_Or);
    BF_RUN_TEST(TestNotXor);
    return 0;
}