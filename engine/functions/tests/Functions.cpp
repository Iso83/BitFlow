#include "TestAssert.h"
#include "common/Expr.h"
#include "common/Rule.h"

#include <BitFlow/engine/functions/FunctionRegistry.h>
#include <BitFlow/engine/io/ExprParser.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Functions;

static ExprRef ExpandCH(FunctionExpandContext& ctx) {
    auto x = ExprRef(ctx.store, ctx.args[0]);
    auto y = ExprRef(ctx.store, ctx.args[1]);
    auto z = ExprRef(ctx.store, ctx.args[2]);

    return (x & y) ^ (~x & z);
}

int Test_FunctionRegistry_DslCall_TracksResultAndParameters() {
    MakeExprStore(32);

    FunctionRegistry reg;

    CPPTEST_ASSERT(reg.Add({.name = "CH", .parameterCount = 3, .expand = ExpandCH}));

    auto fnCH = reg.Get("CH");

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto expr = C(5) + a / fnCH(C(7), C(8), C(7) + C(9));

    CPPTEST_ASSERT(expr.IsValid());

    const auto& calls = reg.Calls();

    CPPTEST_ASSERT(calls.size() == 1);
    CPPTEST_ASSERT(calls[0].name == "CH");
    CPPTEST_ASSERT(calls[0].result.IsValid());
    CPPTEST_ASSERT(calls[0].parameters.size() == 3);
    CPPTEST_ASSERT(calls[0].parameters[0].store == &store);
    CPPTEST_ASSERT(calls[0].parameters[1].store == &store);
    CPPTEST_ASSERT(calls[0].parameters[2].store == &store);
    return 0;
}

int Test_FunctionRegistry_DuplicateName_IsRejected() {
    FunctionRegistry reg;

    CPPTEST_ASSERT(reg.Add({.name = "CH", .parameterCount = 3, .expand = ExpandCH}));
    CPPTEST_ASSERT(!reg.Add({.name = "CH", .parameterCount = 3, .expand = ExpandCH}));
    return 0;
}

int Test_FunctionRegistry_Merge_KeepsUniqueNames() {
    FunctionRegistry a;
    FunctionRegistry b;

    CPPTEST_ASSERT(a.Add({.name = "CH", .parameterCount = 3, .expand = ExpandCH}));
    CPPTEST_ASSERT(b.Add({.name = "CH", .parameterCount = 3, .expand = ExpandCH}));
    CPPTEST_ASSERT(b.Add({.name = "MAJ", .parameterCount = 3, .expand = ExpandCH}));

    a.Merge(b);

    CPPTEST_ASSERT(a.Contains("CH"));
    CPPTEST_ASSERT(a.Contains("MAJ"));
    CPPTEST_ASSERT(a.Find("CH") != nullptr);
    CPPTEST_ASSERT(a.Find("MAJ") != nullptr);
    return 0;
}

int Test_FunctionRegistry_Parse_TracksResultAndParameters() {
    MakeExprStore(32);

    FunctionRegistry reg;

    CPPTEST_ASSERT(reg.Add({.name = "CH", .parameterCount = 3, .expand = ExpandCH}));

    auto parsed = BitFlow::Engine::IO::Parse(&store, "CH(a,b,c)", &reg);

    CPPTEST_ASSERT(parsed.root.IsValid());

    const auto& calls = reg.Calls();

    CPPTEST_ASSERT(calls.size() == 1);
    CPPTEST_ASSERT(calls[0].name == "CH");
    CPPTEST_ASSERT(calls[0].result == parsed.root);
    CPPTEST_ASSERT(calls[0].parameters.size() == 3);

    CPPTEST_ASSERT(parsed.names.at(calls[0].parameters[0].id) == "a");
    CPPTEST_ASSERT(parsed.names.at(calls[0].parameters[1].id) == "b");
    CPPTEST_ASSERT(parsed.names.at(calls[0].parameters[2].id) == "c");

    return 0;
}

int main() {
    CPPTEST_RUN(Test_FunctionRegistry_DslCall_TracksResultAndParameters);
    CPPTEST_RUN(Test_FunctionRegistry_DuplicateName_IsRejected);
    CPPTEST_RUN(Test_FunctionRegistry_Merge_KeepsUniqueNames);
    CPPTEST_RUN(Test_FunctionRegistry_Parse_TracksResultAndParameters);

    return 0;
}
