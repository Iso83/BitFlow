#include "common/Expr.h"
#include "common/String.h"
#include "TestAssert.h"

#include <BitFlow/engine/core/expression/ExprPrinter.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Expression;

int TestToString_PrintsConstant() {
    MakeExprStore(32);

    CPPTEST_ASSERT(ToString(C(123)) == "123");

    return 0;
}

int TestToString_PrintsNamedVariable() {
    MakeExprStore(32);

    CPPTEST_ASSERT(ToString(V("a")) == "a");

    return 0;
}

int TestToString_PrintsUnnamedVariable() {
    MakeExprStore(32);

    CPPTEST_ASSERT(ToString(V()).starts_with("v"));

    return 0;
}

int TestToString_RespectsOperatorPrecedence() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    CPPTEST_ASSERT(ToString(a + (b * c)) == "a + b * c");

    return 0;
}

int TestToString_AddsParensForRightAssociativeCases() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    CPPTEST_ASSERT(ToString(a - (b - c)) == "a - (b - c)");

    return 0;
}

int TestToString_PrintsUnaryOperators() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    CPPTEST_ASSERT(ToString(-(a + b)) == "-(a + b)");
    CPPTEST_ASSERT(ToString(~(a | b)) == "~(a | b)");

    return 0;
}

int TestToString_ExplicitGroupsWrapEverything() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    PrintOptions opts;
    opts.ExplicitGroups();

    CPPTEST_ASSERT(ToString(a + (b * c), opts) == "(a + (b * c))");

    return 0;
}

int TestToString_PowAsFunction() {
    MakeExprStore(32);

    auto a = V("a");

    PrintOptions opts;
    opts.PowAsFunction(true);

    CPPTEST_ASSERT(ToString(a.Pow(C(3)), opts) == "pow(a, 3)");

    return 0;
}

int TestToString_PowAsInfix() {
    MakeExprStore(32);

    auto a = V("a");

    PrintOptions opts;
    opts.PowAsFunction(false);

    CPPTEST_ASSERT(ToString(a.Pow(C(3)), opts) == "a ** 3");

    return 0;
}

int TestToString_ShowExprIds() {
    MakeExprStore(32);

    PrintOptions opts;
    opts.ShowExprIds();

    CPPTEST_ASSERT(Contains(ToString(V("a"), opts), '#'));

    return 0;
}

int TestToString_ShowBitWidth() {
    MakeExprStore(16);

    PrintOptions opts;
    opts.ShowBitWidth();

    CPPTEST_ASSERT(ToString(V("a"), opts) == "a:16");

    return 0;
}

int TestToString_ShowOpTypes() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    PrintOptions opts;
    opts.ShowOpTypes();

    CPPTEST_ASSERT(ToString(a + b, opts) == "Add(a, b)");

    return 0;
}

int TestToString_DebugStructure() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    PrintOptions opts;
    opts.DebugStructure();

    CPPTEST_ASSERT(Contains(ToString(a + b, opts), '('));

    return 0;
}

int TestToString_CombinedDebugOptions() {
    MakeExprStore(8);

    auto a = V("a");

    PrintOptions opts;
    opts.ShowExprIds().ShowBitWidth().ShowOpTypes();

    const std::string s = ToString(~a, opts);

    CPPTEST_ASSERT(Contains(s, "Not"));
    CPPTEST_ASSERT(Contains(s, "#"));

    return 0;
}

int main() {
    CPPTEST_RUN(TestToString_PrintsConstant);
    CPPTEST_RUN(TestToString_PrintsNamedVariable);
    CPPTEST_RUN(TestToString_PrintsUnnamedVariable);
    CPPTEST_RUN(TestToString_RespectsOperatorPrecedence);
    CPPTEST_RUN(TestToString_AddsParensForRightAssociativeCases);
    CPPTEST_RUN(TestToString_PrintsUnaryOperators);
    CPPTEST_RUN(TestToString_ExplicitGroupsWrapEverything);
    CPPTEST_RUN(TestToString_PowAsFunction);
    CPPTEST_RUN(TestToString_PowAsInfix);
    CPPTEST_RUN(TestToString_ShowExprIds);
    CPPTEST_RUN(TestToString_ShowBitWidth);
    CPPTEST_RUN(TestToString_ShowOpTypes);
    CPPTEST_RUN(TestToString_DebugStructure);
    CPPTEST_RUN(TestToString_CombinedDebugOptions);
    return 0;
}
