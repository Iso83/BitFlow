#include "expression/ExprUtils.h"

#include "common/Expr.h"
#include "TestAssert.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Ids;

static int TestContainsExpr_DirectInput() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    auto expr = a ^ b;

    CPPTEST_ASSERT(ContainsExpr(&store, expr.id, a.id));
    CPPTEST_ASSERT(ContainsExpr(&store, expr.id, b.id));

    return 0;
}

static int TestContainsExpr_NotFound() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto expr = a ^ b;

    CPPTEST_ASSERT(!ContainsExpr(&store, expr.id, c.id));

    return 0;
}

static int TestContainsExpr_SelfMatch() {
    MakeExprStore(32);

    auto a = V("a");

    CPPTEST_ASSERT(ContainsExpr(&store, a.id, a.id));

    return 0;
}

static int TestMatchZero_Positive() {
    MakeExprStore(32);

    auto a = V("a");
    auto zero = C(0);

    auto expr = a + zero;

    CPPTEST_ASSERT(Match_Zero<OpType::Add>(&store, &names, expr.id));

    return 0;
}

static int TestMatchZero_WrongOp() {
    MakeExprStore(32);

    auto a = V("a");
    auto zero = C(0);

    auto expr = a ^ zero;

    CPPTEST_ASSERT(!Match_Zero<OpType::Add>(&store, &names, expr.id));

    return 0;
}

static int TestMatchZero_NoZeroInput() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    auto expr = a + b;

    CPPTEST_ASSERT(!Match_Zero<OpType::Add>(&store, &names, expr.id));

    return 0;
}

static int TestMakeXor_EmptyTerms() {
    MakeExprStore(32);

    ExprInputs terms{};

    auto out = store.create(OpType::Xor, std::move(terms), 32);

    CPPTEST_ASSERT(InputSize(out) == 0);

    return 0;
}

static int TestMakeXor_SingleTerm() {
    MakeExprStore(32);

    auto a = V("a");

    ExprInputs terms{a.id};

    auto out = store.create(OpType::Xor, std::move(terms), 32);

    CPPTEST_ASSERT(Input(out, 0) == a);

    return 0;
}

static int TestMakeXor_MultipleTerms() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    ExprInputs terms{a.id, b.id};

    auto out = store.create(OpType::Xor, std::move(terms), 32);

    CPPTEST_ASSERT(Op(out) == OpType::Xor);
    CPPTEST_ASSERT(InputSize(out) == 2);

    CPPTEST_ASSERT(Input(out, 0) == a);
    CPPTEST_ASSERT(Input(out, 1) == b);

    return 0;
}

static int TestCompareExprCanonical_SameExpr() {
    MakeExprStore(32);

    auto a = V("a");

    CPPTEST_ASSERT(CompareExprCanonical(&store, &names, a.id, a.id) == 0);

    return 0;
}

static int TestCompareExprCanonical_ConstantOrder() {
    MakeExprStore(32);

    auto c1 = C(1);
    auto c2 = C(2);

    CPPTEST_ASSERT(CompareExprCanonical(&store, &names, c1.id, c2.id) < 0);
    CPPTEST_ASSERT(CompareExprCanonical(&store, &names, c2.id, c1.id) > 0);

    return 0;
}

static int TestCompareExprCanonical_ArityOrder() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto x1 = a ^ b;
    auto x2 = a ^ b ^ c;

    CPPTEST_ASSERT(CompareExprCanonical(&store, &names, x1.id, x2.id) < 0);

    return 0;
}

static int TestCanonicalExprLess_Positive() {
    MakeExprStore(32);

    auto c1 = C(1);
    auto c2 = C(2);

    CPPTEST_ASSERT(CanonicalExprLess(&store, &names, c1.id, c2.id));
    CPPTEST_ASSERT(!CanonicalExprLess(&store, &names, c2.id, c1.id));

    return 0;
}

int main() {
    CPPTEST_RUN(TestContainsExpr_DirectInput);
    CPPTEST_RUN(TestContainsExpr_NotFound);
    CPPTEST_RUN(TestContainsExpr_SelfMatch);

    CPPTEST_RUN(TestMatchZero_Positive);
    CPPTEST_RUN(TestMatchZero_WrongOp);
    CPPTEST_RUN(TestMatchZero_NoZeroInput);

    CPPTEST_RUN(TestMakeXor_EmptyTerms);
    CPPTEST_RUN(TestMakeXor_SingleTerm);
    CPPTEST_RUN(TestMakeXor_MultipleTerms);

    CPPTEST_RUN(TestCompareExprCanonical_SameExpr);
    CPPTEST_RUN(TestCompareExprCanonical_ConstantOrder);
    CPPTEST_RUN(TestCompareExprCanonical_ArityOrder);

    CPPTEST_RUN(TestCanonicalExprLess_Positive);

    return 0;
}
