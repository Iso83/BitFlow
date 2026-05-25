#include <BitFlow/functions/FunctionRegistry.h>
#include <BitFlow/io/ExprParser.h>
#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Expression;
using namespace BitFlow::Functions;
using namespace BitFlow::Testing;

static ExprRef ExpandCH(FunctionExpandContext& ctx) {
    auto x = ExprRef(ctx.store, ctx.args[0]);
    auto y = ExprRef(ctx.store, ctx.args[1]);
    auto z = ExprRef(ctx.store, ctx.args[2]);

    return (x & y) ^ (~x & z);
}

int Test_FunctionRegistry_DslCall_TracksResultAndParameters() {
    MakeExprStore(32);

    FunctionRegistry reg;

    BF_TEST(reg.Add({.name = "CH", .parameterCount = 3, .expand = ExpandCH}));

    auto fnCH = reg.Get("CH");

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto expr = C(5) + a / fnCH(C(7), C(8), C(7) + C(9));

    BF_TEST(expr.IsValid());

    const auto& calls = reg.Calls();

    BF_TEST(calls.size() == 1);
    BF_TEST(calls[0].name == "CH");
    BF_TEST(calls[0].result.IsValid());
    BF_TEST(calls[0].parameters.size() == 3);
    BF_TEST(calls[0].parameters[0].store == &store);
    BF_TEST(calls[0].parameters[1].store == &store);
    BF_TEST(calls[0].parameters[2].store == &store);
    return 0;
}

int Test_FunctionRegistry_DuplicateName_IsRejected() {
    FunctionRegistry reg;

    BF_TEST(reg.Add({.name = "CH", .parameterCount = 3, .expand = ExpandCH}));
    BF_TEST(!reg.Add({.name = "CH", .parameterCount = 3, .expand = ExpandCH}));
    return 0;
}

int Test_FunctionRegistry_Merge_KeepsUniqueNames() {
    FunctionRegistry a;
    FunctionRegistry b;

    BF_TEST(a.Add({.name = "CH", .parameterCount = 3, .expand = ExpandCH}));
    BF_TEST(b.Add({.name = "CH", .parameterCount = 3, .expand = ExpandCH}));
    BF_TEST(b.Add({.name = "MAJ", .parameterCount = 3, .expand = ExpandCH}));

    a.Merge(b);

    BF_TEST(a.Contains("CH"));
    BF_TEST(a.Contains("MAJ"));
    BF_TEST(a.Find("CH") != nullptr);
    BF_TEST(a.Find("MAJ") != nullptr);
    return 0;
}

int Test_FunctionRegistry_Parse_TracksResultAndParameters() {
    MakeExprStore(32);

    FunctionRegistry reg;

    BF_TEST(reg.Add({.name = "CH", .parameterCount = 3, .expand = ExpandCH}));

    auto parsed = BitFlow::IO::Parse(&store, "CH(a,b,c)", &reg);

    BF_TEST(parsed.root.IsValid());

    const auto& calls = reg.Calls();

    BF_TEST(calls.size() == 1);
    BF_TEST(calls[0].name == "CH");
    BF_TEST(calls[0].result == parsed.root);
    BF_TEST(calls[0].parameters.size() == 3);

    BF_TEST(parsed.names.at(calls[0].parameters[0].id) == "a");
    BF_TEST(parsed.names.at(calls[0].parameters[1].id) == "b");
    BF_TEST(parsed.names.at(calls[0].parameters[2].id) == "c");

    return 0;
}

int main() {
    BF_RUN_TEST(Test_FunctionRegistry_DslCall_TracksResultAndParameters);
    BF_RUN_TEST(Test_FunctionRegistry_DuplicateName_IsRejected);
    BF_RUN_TEST(Test_FunctionRegistry_Merge_KeepsUniqueNames);
    BF_RUN_TEST(Test_FunctionRegistry_Parse_TracksResultAndParameters);

    return 0;
}