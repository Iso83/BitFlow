#include <BitFlow/core/rules/RuleEngine.h>
#include <ExprTestUtils.h>
#include <RuleTestHelpers.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Rules;

int TestAddRule_NoDuplicates() {
    RuleEngine engine;

    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Flatten_Rule());

    BF_VALIDATE_ENGINE(engine, Normalize::Get_Flatten_Rule());
    return 0;
}

int TestMerge_NoDuplicates() {
    RuleEngine a;
    a.AddRule(Normalize::Get_Flatten_Rule());

    RuleEngine b;
    b.AddRule(Normalize::Get_Flatten_Rule());

    a.Merge(b);

    BF_VALIDATE_ENGINE(a, Normalize::Get_Flatten_Rule());
    return 0;
}

int TestValidateDependencies_MissingRule() {
    RuleEngine engine;
    engine.AddRule(Simplify::Bitwise::Get_XorAndReduction_Rule());

    auto valResult = engine.AnalyzeDependencies(Simplify::Bitwise::Get_XorAndReduction_Rule());

    BF_TEST(valResult.missing.size() > 0);
    return 0;
}

int TestValidateDependencies_OrderViolation() {
    RuleEngine engine;

    engine.AddRule(Simplify::Bitwise::Get_XorAndReduction_Rule());

    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());
    engine.AddRule(Simplify::Bitwise::Get_AndXorReduction_Rule());

    bool failed = false;

    try {
        MakeExprStore(32);

        auto x = V("x");
        Rewrite(engine, x);
    } catch (...) {
        failed = true;
    }

    BF_TEST(failed);
    return 0;
}

int TestRewrite_FixedPoint() {
    MakeExprStore(32);

    RuleEngine engine;

    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());

    engine.AddRule(Simplify::Bitwise::Get_XorZero_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorCancel_Rule());

    BF_VALIDATE_ENGINE(engine, Simplify::Bitwise::Get_XorCancel_Rule());

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x ^ x ^ x ^ x));

    BF_TEST(IsFalse(r));
    return 0;
}

int TestDebugCallback() {
    MakeExprStore(32);

    RuleEngine engine;

    engine.AddRule(Normalize::Get_Flatten_Rule());
    engine.AddRule(Normalize::Get_Order_Rule());

    engine.AddRule(Simplify::Bitwise::Get_XorZero_Rule());
    engine.AddRule(Simplify::Bitwise::Get_XorCancel_Rule());

    BF_VALIDATE_ENGINE(engine, Simplify::Bitwise::Get_XorCancel_Rule());

    bool callbackCalled = false;
    bool beginCalled = false;
    bool endCalled = false;
    bool validCallback = true;
    bool ctxStore = false;

    engine.SetDebugCallback([&](RuleEngine::DebugCallBack_Ctx& debugCtx) {
        callbackCalled = true;

        if (!(debugCtx.key == Simplify::Bitwise::XorCancel))
            validCallback = false;

        if (debugCtx.store)
            ctxStore = true;

        debugCtx.beginCallback = [&](ExprId id) { beginCalled = true; };

        debugCtx.endCallback = [&](ExprId id) { endCalled = true; };
    });

    auto x = V("x");

    BF_SAFE_REWRITE(r, Rewrite(engine, x ^ x));

    BF_TEST(callbackCalled);
    BF_TEST(beginCalled);
    BF_TEST(endCalled);
    BF_TEST(validCallback);
    BF_TEST(ctxStore);

    BF_TEST(IsFalse(r));

    return 0;
}

int main() {
    BF_RUN_TEST(TestAddRule_NoDuplicates);
    BF_RUN_TEST(TestMerge_NoDuplicates);

    BF_RUN_TEST(TestValidateDependencies_MissingRule);
    BF_RUN_TEST(TestValidateDependencies_OrderViolation);

    BF_RUN_TEST(TestRewrite_FixedPoint);
    BF_RUN_TEST(TestDebugCallback);

    return 0;
}