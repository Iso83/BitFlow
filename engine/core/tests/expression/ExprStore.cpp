#include "TestAssert.h"
#include "common/Expr.h"

using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Testing;

int TestMakeFalse_CreatesFalseConstant() {
    MakeExprStore(32);

    auto v = store.makeFalse(32);

    CPPTEST_ASSERT(v.IsValid());
    CPPTEST_ASSERT(Op(v) == OpType::Const);
    CPPTEST_ASSERT(ExprOf(v).inputs.empty());
    CPPTEST_ASSERT(BitWidth(v) == 32);
    CPPTEST_ASSERT(ExprOf(v).knownMask == Expr::fullMask(32));
    CPPTEST_ASSERT(EqualChunkValue(v, 0));

    CPPTEST_ASSERT(store.isFalse(v.id));
    CPPTEST_ASSERT(!store.isTrue(v.id));

    return 0;
}

int TestMakeTrue_CreatesTrueConstant() {
    MakeExprStore(16);

    auto v = store.makeTrue(16);

    CPPTEST_ASSERT(v.IsValid());

    CPPTEST_ASSERT(Op(v) == OpType::Const);
    CPPTEST_ASSERT(ExprOf(v).inputs.empty());
    CPPTEST_ASSERT(BitWidth(v) == 16);
    CPPTEST_ASSERT(ExprOf(v).knownMask == Expr::fullMask(16));
    CPPTEST_ASSERT(EqualChunkValue(v, Expr::fullMask(16)));

    CPPTEST_ASSERT(store.isTrue(v.id));
    CPPTEST_ASSERT(!store.isFalse(v.id));

    return 0;
}

int TestInvertConst_InvertsConstantValue() {
    MakeExprStore(8);

    auto c = C(0b10101010);
    auto inv = store.invertConst(c.id);

    const Expr& expr = store[inv];

    CPPTEST_ASSERT(expr.op == OpType::Const);
    CPPTEST_ASSERT(expr.bitWidth == 8);

    CPPTEST_ASSERT(expr.knownValue == 0b01010101);
    CPPTEST_ASSERT(expr.knownMask == Expr::fullMask(8));

    return 0;
}

int TestContains_ReturnsTrueForAliveExpr() {
    MakeExprStore(32);

    auto v = V("x");

    CPPTEST_ASSERT(store.contains(v));

    return 0;
}

int TestRemove_RemovesExprFromStore() {
    MakeExprStore(32);

    auto v = V("x");

    CPPTEST_ASSERT(store.contains(v));
    CPPTEST_ASSERT(store.remove(v));
    CPPTEST_ASSERT(!store.contains(v));

    return 0;
}

int TestRemove_RejectsDoubleRemove() {
    MakeExprStore(32);

    auto v = V("x");

    CPPTEST_ASSERT(store.remove(v));
    CPPTEST_ASSERT(!store.remove(v));

    return 0;
}

int TestContains_ReturnsFalseAfterRemove() {
    MakeExprStore(32);

    auto v = V("x");

    CPPTEST_ASSERT(store.contains(v));

    auto removed = store.remove(v);

    CPPTEST_ASSERT(!store.contains(v));

    return 0;
}

int Test_StructuralEquivalent_CommutativeOrder() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");
    auto c = V("c");

    auto lhs = a + b + c;
    auto rhs = c + a + b;

    CPPTEST_ASSERT(store.structuralEquivalent(lhs.id, rhs.id));

    return 0;
}

int Test_StructuralEquivalent_NonCommutativeOrder() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    auto lhs = a - b;
    auto rhs = b - a;

    CPPTEST_ASSERT(!store.structuralEquivalent(lhs.id, rhs.id));

    return 0;
}

int Test_StructuralEquivalent_DuplicateInputs() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    auto lhs = a + a + b;
    auto rhs = a + b + b;

    CPPTEST_ASSERT(!store.structuralEquivalent(lhs.id, rhs.id));

    return 0;
}

int Test_EqualConstValue_DifferentBitWidths() {
    MakeExprStore(128);

    auto a = store.createConstant(3, 2);
    auto b = store.createConstant(3, 128);

    CPPTEST_ASSERT(store.equalConstValue(a, b));

    return 0;
}

int Test_EqualConstValue_DifferentValues() {
    MakeExprStore(128);

    auto a = store.createConstant(3, 32);
    auto b = store.createConstant(4, 128);

    CPPTEST_ASSERT(!store.equalConstValue(a, b));

    return 0;
}

int main() {
    CPPTEST_RUN(TestMakeFalse_CreatesFalseConstant);
    CPPTEST_RUN(TestMakeTrue_CreatesTrueConstant);
    CPPTEST_RUN(TestInvertConst_InvertsConstantValue);
    CPPTEST_RUN(TestContains_ReturnsTrueForAliveExpr);
    CPPTEST_RUN(TestRemove_RemovesExprFromStore);
    CPPTEST_RUN(TestRemove_RejectsDoubleRemove);
    CPPTEST_RUN(TestContains_ReturnsFalseAfterRemove);

    CPPTEST_RUN(Test_StructuralEquivalent_CommutativeOrder);
    CPPTEST_RUN(Test_StructuralEquivalent_NonCommutativeOrder);
    CPPTEST_RUN(Test_StructuralEquivalent_DuplicateInputs);

    CPPTEST_RUN(Test_EqualConstValue_DifferentBitWidths);
    CPPTEST_RUN(Test_EqualConstValue_DifferentValues);

    return 0;
}
