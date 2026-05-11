#include <BitFlow/core/expression/ExprPrinter.h>
#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Expression;

int TestToString_PrintsConstant() {
    MakeExprStore(32);

    BF_TEST(ToString(C(123)) == "123");

    return 0;
}

int TestToString_PrintsNamedVariable() {
    MakeExprStore(32);

    BF_TEST(ToString(V("a")) == "a");

    return 0;
}

int TestToString_PrintsUnnamedVariable() {
    MakeExprStore(32);

    BF_TEST(ToString(V()).starts_with("v"));

    return 0;
}

int TestToString_RespectsOperatorPrecedence() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_TEST(ToString(a + (b * c)) == "a + b * c");

    return 0;
}

int TestToString_AddsParensForRightAssociativeCases() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    BF_TEST(ToString(a - (b - c)) == "a - (b - c)");

    return 0;
}

int TestToString_PrintsUnaryOperators() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    BF_TEST(ToString(-(a + b)) == "-(a + b)");
    BF_TEST(ToString(~(a | b)) == "~(a | b)");

    return 0;
}

int TestToString_ExplicitGroupsWrapEverything() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    PrintOptions opts;
    opts.ExplicitGroups();

    BF_TEST(ToString(a + (b * c), opts) == "(a + (b * c))");

    return 0;
}

int TestToString_RotlAsFunction() {
    MakeExprStore(32);

    auto a = V("a");

    PrintOptions opts;
    opts.RotAsFunction(true);

    BF_TEST(ToString(a.RotL(C(3)), opts) == "rotl(a, 3)");

    return 0;
}

int TestToString_RotlAsInfix() {
    MakeExprStore(32);

    auto a = V("a");

    PrintOptions opts;
    opts.RotAsFunction(false);

    BF_TEST(ToString(a.RotL(C(3)), opts) == "a <<< 3");

    return 0;
}

int TestToString_ShowExprIds() {
    MakeExprStore(32);

    PrintOptions opts;
    opts.ShowExprIds();

    BF_TEST(Contains(ToString(V("a"), opts), '#'));

    return 0;
}

int TestToString_ShowBitWidth() {
    MakeExprStore(16);

    PrintOptions opts;
    opts.ShowBitWidth();

    BF_TEST(ToString(V("a"), opts) == "a:16");

    return 0;
}

int TestToString_ShowOpTypes() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    PrintOptions opts;
    opts.ShowOpTypes();

    BF_TEST(ToString(a + b, opts) == "Add(a, b)");

    return 0;
}

int TestToString_DebugStructure() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    PrintOptions opts;
    opts.DebugStructure();

    BF_TEST(Contains(ToString(a + b, opts), '('));

    return 0;
}

int TestToString_CombinedDebugOptions() {
    MakeExprStore(8);

    auto a = V("a");

    PrintOptions opts;
    opts.ShowExprIds().ShowBitWidth().ShowOpTypes();

    const std::string s = ToString(~a, opts);

    BF_TEST(Contains(s, "Not"));
    BF_TEST(Contains(s, "#"));

    return 0;
}

int main() {
    BF_RUN_TEST(TestToString_PrintsConstant);
    BF_RUN_TEST(TestToString_PrintsNamedVariable);
    BF_RUN_TEST(TestToString_PrintsUnnamedVariable);
    BF_RUN_TEST(TestToString_RespectsOperatorPrecedence);
    BF_RUN_TEST(TestToString_AddsParensForRightAssociativeCases);
    BF_RUN_TEST(TestToString_PrintsUnaryOperators);
    BF_RUN_TEST(TestToString_ExplicitGroupsWrapEverything);
    BF_RUN_TEST(TestToString_RotlAsFunction);
    BF_RUN_TEST(TestToString_RotlAsInfix);
    BF_RUN_TEST(TestToString_ShowExprIds);
    BF_RUN_TEST(TestToString_ShowBitWidth);
    BF_RUN_TEST(TestToString_ShowOpTypes);
    BF_RUN_TEST(TestToString_DebugStructure);
    BF_RUN_TEST(TestToString_CombinedDebugOptions);
    return 0;
}