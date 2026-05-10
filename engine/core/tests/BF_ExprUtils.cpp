#include "expression/ExprUtils.h"

#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Ids;

static int TestContainsExpr_DirectInput() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    auto expr = a ^ b;

    BF_TEST(ContainsExpr(&store, expr.id, a.id));
    BF_TEST(ContainsExpr(&store, expr.id, b.id));

    return 0;
}

static int TestContainsExpr_NotFound() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto expr = a ^ b;

    BF_TEST(!ContainsExpr(&store, expr.id, c.id));

    return 0;
}

static int TestContainsExpr_SelfMatch() {
    MakeExprStore(32);

    auto a = V("a");

    BF_TEST(ContainsExpr(&store, a.id, a.id));

    return 0;
}

static int TestMatchZero_Positive() {
    MakeExprStore(32);

    auto a = V("a");
    auto zero = C(0);

    auto expr = a + zero;

    BF_TEST(Match_Zero<OpType::Add>(&store, expr.id));

    return 0;
}

static int TestMatchZero_WrongOp() {
    MakeExprStore(32);

    auto a = V("a");
    auto zero = C(0);

    auto expr = a ^ zero;

    BF_TEST(!Match_Zero<OpType::Add>(&store, expr.id));

    return 0;
}

static int TestMatchZero_NoZeroInput() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    auto expr = a + b;

    BF_TEST(!Match_Zero<OpType::Add>(&store, expr.id));

    return 0;
}

static int TestMakeXor_EmptyTerms() {
    MakeExprStore(32);

    std::vector<ExprId> terms{};

    auto out = MakeXor(&store, terms);

    BF_TEST(EqualChunkValue(ERef(out), 0u));

    return 0;
}

static int TestMakeXor_SingleTerm() {
    MakeExprStore(32);

    auto a = V("a");

    std::vector<ExprId> terms{a.id};

    auto out = MakeXor(&store, terms);

    BF_TEST(out == a.id);

    return 0;
}

static int TestMakeXor_MultipleTerms() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    std::vector<ExprId> terms{a.id, b.id};

    const Expr& out = store[MakeXor(&store, terms)];

    BF_TEST(out.op == OpType::Xor);
    BF_TEST(out.inputs.size() == 2);

    BF_TEST(ERef(out.inputs[0]) == a);
    BF_TEST(ERef(out.inputs[1]) == b);

    return 0;
}

static int TestCompareExprCanonical_SameExpr() {
    MakeExprStore(32);

    auto a = V("a");

    BF_TEST(CompareExprCanonical(&store, a.id, a.id) == 0);

    return 0;
}

static int TestCompareExprCanonical_ConstantOrder() {
    MakeExprStore(32);

    auto c1 = C(1);
    auto c2 = C(2);

    BF_TEST(CompareExprCanonical(&store, c1.id, c2.id) < 0);
    BF_TEST(CompareExprCanonical(&store, c2.id, c1.id) > 0);

    return 0;
}

static int TestCompareExprCanonical_ArityOrder() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto x1 = a ^ b;
    auto x2 = a ^ b ^ c;

    BF_TEST(CompareExprCanonical(&store, x1.id, x2.id) < 0);

    return 0;
}

static int TestCanonicalExprLess_Positive() {
    MakeExprStore(32);

    auto c1 = C(1);
    auto c2 = C(2);

    BF_TEST(CanonicalExprLess(&store, c1.id, c2.id));
    BF_TEST(!CanonicalExprLess(&store, c2.id, c1.id));

    return 0;
}

int main() {
    BF_RUN_TEST(TestContainsExpr_DirectInput);
    BF_RUN_TEST(TestContainsExpr_NotFound);
    BF_RUN_TEST(TestContainsExpr_SelfMatch);

    BF_RUN_TEST(TestMatchZero_Positive);
    BF_RUN_TEST(TestMatchZero_WrongOp);
    BF_RUN_TEST(TestMatchZero_NoZeroInput);

    BF_RUN_TEST(TestMakeXor_EmptyTerms);
    BF_RUN_TEST(TestMakeXor_SingleTerm);
    BF_RUN_TEST(TestMakeXor_MultipleTerms);

    BF_RUN_TEST(TestCompareExprCanonical_SameExpr);
    BF_RUN_TEST(TestCompareExprCanonical_ConstantOrder);
    BF_RUN_TEST(TestCompareExprCanonical_ArityOrder);

    BF_RUN_TEST(TestCanonicalExprLess_Positive);

    return 0;
}