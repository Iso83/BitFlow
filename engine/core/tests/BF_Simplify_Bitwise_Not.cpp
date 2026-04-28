#include <BitFlow/core/rules/Rule.h>
#include <BitFlow/core/rules/RuleEngine.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

int TestNotDoubleNegation() {
    auto x = MakeVar(1);

    auto n1 = MakeOp(2, OpType::Not, {x});
    auto n2 = MakeOp(3, OpType::Not, {n1});

    RuleEngine engine;
    engine.AddRule(Simplify::Bitwise::Get_Not_Rule());

    ExprOld* r = engine.Rewrite(n2);

    BF_TEST(r->id == x->id);
    return 0;
}

int TestNotConst() {
    auto c = MakeConst(1, 0b1010);

    auto expr = MakeOp(2, OpType::Not, {c});

    RuleEngine engine;
    engine.AddRule(Simplify::Bitwise::Get_Not_Rule());

    ExprOld* r = engine.Rewrite(expr);

    BF_TEST(r->op == OpType::Const);
    BF_TEST(r->constValue == ~0b1010);
    return 0;
}

int TestNotPushdown_And() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);

    auto inner = MakeOp(3, OpType::And, {a, b});
    auto expr = MakeOp(4, OpType::Not, {inner});

    RuleEngine engine;
    engine.AddRule(Simplify::Bitwise::Get_NotPushdown_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Not_Rule());

    ExprOld* r = engine.Rewrite(expr);

    BF_TEST(r->op == OpType::Or);
    BF_TEST(r->inputs.size() == 2);
    return 0;
}

int TestNotPushdown_Or() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);

    auto inner = MakeOp(3, OpType::Or, {a, b});
    auto expr = MakeOp(4, OpType::Not, {inner});

    RuleEngine engine;
    engine.AddRule(Simplify::Bitwise::Get_NotPushdown_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Not_Rule());

    ExprOld* r = engine.Rewrite(expr);

    BF_TEST(r->op == OpType::And);
    BF_TEST(r->inputs.size() == 2);
    return 0;
}

int TestNotXor() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);

    auto inner = MakeOp(3, OpType::Xor, {a, b});
    auto expr = MakeOp(4, OpType::Not, {inner});

    RuleEngine engine;
    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Simplify::Bitwise::Get_Not_Xor_Rule());

    ExprOld* r = engine.Rewrite(expr);

    BF_TEST(r->op == OpType::Xor);
    BF_TEST(r->inputs.size() == 3);
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