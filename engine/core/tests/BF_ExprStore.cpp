#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Expression;
using namespace BitFlow::Testing;

int TestMakeFalse_CreatesFalseConstant() {
    MakeExprStore(32);

    auto v = store.makeFalse(32);

    BF_TEST(v.IsValid());
    BF_TEST(Op(v) == OpType::Const);
    BF_TEST(ExprOf(v).inputs.empty());
    BF_TEST(BitWidth(v) == 32);
    BF_TEST(ExprOf(v).knownMask == Expr::fullMask(32));
    BF_TEST(EqualChunkValue(v, 0));

    BF_TEST(store.isFalse(v.id));
    BF_TEST(!store.isTrue(v.id));

    return 0;
}

int TestMakeTrue_CreatesTrueConstant() {
    MakeExprStore(16);

    auto v = store.makeTrue(16);

    BF_TEST(v.IsValid());

    BF_TEST(Op(v) == OpType::Const);
    BF_TEST(ExprOf(v).inputs.empty());
    BF_TEST(BitWidth(v) == 16);
    BF_TEST(ExprOf(v).knownMask == Expr::fullMask(16));
    BF_TEST(EqualChunkValue(v, Expr::fullMask(16)));

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

    auto removed = store.remove(v);

    BF_TEST(!store.contains(v));

    return 0;
}

int Test_StructuralEquivalent_CommutativeOrder() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto lhs = a + b + c;
    auto rhs = c + a + b;

    BF_TEST(store.structuralEquivalent(lhs.id, rhs.id));

    return 0;
}

int Test_StructuralEquivalent_NonCommutativeOrder() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    auto lhs = a - b;
    auto rhs = b - a;

    BF_TEST(!store.structuralEquivalent(lhs.id, rhs.id));

    return 0;
}

int Test_StructuralEquivalent_DuplicateInputs() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    auto lhs = a + a + b;
    auto rhs = a + b + b;

    BF_TEST(!store.structuralEquivalent(lhs.id, rhs.id));

    return 0;
}

int Test_EqualConstValue_DifferentBitWidths() {
    MakeExprStore(128);

    auto a = store.createConstant(3, 2);
    auto b = store.createConstant(3, 128);

    BF_TEST(store.equalConstValue(a, b));

    return 0;
}

int Test_EqualConstValue_DifferentValues() {
    MakeExprStore(128);

    auto a = store.createConstant(3, 32);
    auto b = store.createConstant(4, 128);

    BF_TEST(!store.equalConstValue(a, b));

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

    BF_RUN_TEST(Test_StructuralEquivalent_CommutativeOrder);
    BF_RUN_TEST(Test_StructuralEquivalent_NonCommutativeOrder);
    BF_RUN_TEST(Test_StructuralEquivalent_DuplicateInputs);

    BF_RUN_TEST(Test_EqualConstValue_DifferentBitWidths);
    BF_RUN_TEST(Test_EqualConstValue_DifferentValues);

    return 0;
}