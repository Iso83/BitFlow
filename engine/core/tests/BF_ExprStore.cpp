#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Testing;

int TestMakeFalse_CreatesFalseConstant() {
    MakeExprStore(32);

    auto v = store.makeFalse(32);

    BF_TEST(v.IsValid());

    const Expr& expr = store[v];

    BF_TEST(expr.op == OpType::Const);
    BF_TEST(expr.inputs.empty());
    BF_TEST(expr.bitWidth == 32);
    BF_TEST(expr.knownMask == Expr::fullMask(32));
    BF_TEST(expr.knownValue == 0);

    BF_TEST(store.isFalse(v.id));
    BF_TEST(!store.isTrue(v.id));

    return 0;
}

int TestMakeTrue_CreatesTrueConstant() {
    MakeExprStore(16);

    auto v = store.makeTrue(16);

    BF_TEST(v.IsValid());

    const Expr& expr = store[v];

    BF_TEST(expr.op == OpType::Const);
    BF_TEST(expr.inputs.empty());
    BF_TEST(expr.bitWidth == 16);
    BF_TEST(expr.knownMask == Expr::fullMask(16));
    BF_TEST(expr.knownValue == Expr::fullMask(16));

    BF_TEST(store.isTrue(v.id));
    BF_TEST(!store.isFalse(v.id));

    return 0;
}

int TestInvertConst_InvertsConstantValue() {
    MakeExprStore(8);

    auto c = C(0b10101010);
    auto inv = store.invertConst(c.id);

    const Expr& expr = store[inv];

    BF_TEST(expr.op == OpType::Const);
    BF_TEST(expr.bitWidth == 8);

    BF_TEST(expr.knownValue == 0b01010101);
    BF_TEST(expr.knownMask == Expr::fullMask(8));

    return 0;
}

int TestContains_ReturnsTrueForAliveExpr() {
    MakeExprStore(32);

    auto v = V("x");

    BF_TEST(store.contains(v));

    return 0;
}

int TestRemove_RemovesExprFromStore() {
    MakeExprStore(32);

    auto v = V("x");

    BF_TEST(store.contains(v));
    BF_TEST(store.remove(v));
    BF_TEST(!store.contains(v));

    return 0;
}

int TestRemove_RejectsDoubleRemove() {
    MakeExprStore(32);

    auto v = V("x");

    BF_TEST(store.remove(v));
    BF_TEST(!store.remove(v));

    return 0;
}

int TestContains_ReturnsFalseAfterRemove() {
    MakeExprStore(32);

    auto v = V("x");

    BF_TEST(store.contains(v));

    store.remove(v);

    BF_TEST(!store.contains(v));

    return 0;
}

int main() {
    BF_RUN_TEST(TestMakeFalse_CreatesFalseConstant);
    BF_RUN_TEST(TestMakeTrue_CreatesTrueConstant);
    BF_RUN_TEST(TestInvertConst_InvertsConstantValue);
    BF_RUN_TEST(TestContains_ReturnsTrueForAliveExpr);
    BF_RUN_TEST(TestRemove_RemovesExprFromStore);
    BF_RUN_TEST(TestRemove_RejectsDoubleRemove);
    BF_RUN_TEST(TestContains_ReturnsFalseAfterRemove);

    return 0;
}