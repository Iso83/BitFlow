#include "common/Expr.h"
#include "common/Rule.h"
#include "TestAssert.h"

#include <BitFlow/engine/core/rules/RuleEngine.h>
#include <BitFlow/engine/core/rules/RulePipeline.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Ids;
using namespace BitFlow::Engine::Core::Rules;

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

    CPPTEST_ASSERT(valResult.missing.size() > 0);
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
        BF_REWRITE(x);
    } catch (...) {
        failed = true;
    }

    CPPTEST_ASSERT(failed);
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

    BF_SAFE_REWRITE(r, BF_REWRITE(x ^ x ^ x ^ x));

    CPPTEST_ASSERT(IsFalse(r));
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

    BF_SAFE_REWRITE(r, BF_REWRITE(x ^ x));

    CPPTEST_ASSERT(callbackCalled);
    CPPTEST_ASSERT(beginCalled);
    CPPTEST_ASSERT(endCalled);
    CPPTEST_ASSERT(validCallback);
    CPPTEST_ASSERT(ctxStore);

    CPPTEST_ASSERT(IsFalse(r));

    return 0;
}

int TestCollectRequiredRules_AddZero() {
    RuleEngine engine = BuildExplore();

    auto rules = engine.CollectRequiredRules(Simplify::Arithmetic::Get_AddZero_Rule().key);

    CPPTEST_ASSERT(rules.contains(Simplify::Arithmetic::Get_AddZero_Rule().key));

    CPPTEST_ASSERT(rules.contains(Normalize::Get_Order_Rule().key));

    return 0;
}

int TestCollectRequiredRules_MulOne() {
    RuleEngine engine = BuildExplore();

    auto rules = engine.CollectRequiredRules(Simplify::Arithmetic::Get_MulOne_Rule().key);

    CPPTEST_ASSERT(rules.contains(Simplify::Arithmetic::Get_MulOne_Rule().key));

    CPPTEST_ASSERT(rules.contains(Normalize::Get_Order_Rule().key));

    return 0;
}

int TestCollectRequiredRules_SubZero() {
    RuleEngine engine = BuildExplore();

    auto rules = engine.CollectRequiredRules(Simplify::Arithmetic::Get_SubZero_Rule().key);

    CPPTEST_ASSERT(rules.contains(Simplify::Arithmetic::Get_SubZero_Rule().key));

    CPPTEST_ASSERT(!rules.contains(Normalize::Get_Order_Rule().key));

    return 0;
}

int TestCollectRequiredRules_Recursive() {
    RuleEngine engine = BuildExplore();

    auto rules = engine.CollectRequiredRules(Factorize::Arithmetic::Get_AddCommonFactor_Rule().key);

    CPPTEST_ASSERT(rules.contains(Factorize::Arithmetic::Get_AddCommonFactor_Rule().key));

    CPPTEST_ASSERT(rules.contains(Normalize::Get_Order_Rule().key));

    CPPTEST_ASSERT(rules.contains(Normalize::Get_Flatten_Rule().key));

    return 0;
}

int main() {
    CPPTEST_RUN(TestAddRule_NoDuplicates);
    CPPTEST_RUN(TestMerge_NoDuplicates);

    CPPTEST_RUN(TestValidateDependencies_MissingRule);
    CPPTEST_RUN(TestValidateDependencies_OrderViolation);

    CPPTEST_RUN(TestRewrite_FixedPoint);
    CPPTEST_RUN(TestDebugCallback);

    CPPTEST_RUN(TestCollectRequiredRules_AddZero);
    CPPTEST_RUN(TestCollectRequiredRules_MulOne);
    CPPTEST_RUN(TestCollectRequiredRules_SubZero);
    CPPTEST_RUN(TestCollectRequiredRules_Recursive);

    return 0;
}
