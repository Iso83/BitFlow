#include <BitFlow/core/eval/ConstantDetect.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Eval;
using namespace BitFlow::Core::Expression;

int TestDetect_NullIsFalse() {
    BF_TEST(!IsFullyConstant(nullptr));
    return 0;
}

int TestDetect_ConstLeaf() {
    auto c = MakeConst(1, 42);
    BF_TEST(IsFullyConstant(c));
    return 0;
}

int TestDetect_VarLeafFalse() {
    auto x = MakeVar(1);
    BF_TEST(!IsFullyConstant(x));
    return 0;
}

int TestDetect_AllChildrenConstant() {
    auto c1 = MakeConst(1, 10);
    auto c2 = MakeConst(2, 20);
    auto expr = MakeOp(3, OpType::Add, {c1, c2});

    BF_TEST(IsFullyConstant(expr));
    return 0;
}

int TestDetect_AnyChildNonConstantFalse() {
    auto c = MakeConst(1, 10);
    auto x = MakeVar(2);
    auto expr = MakeOp(3, OpType::Add, {c, x});

    BF_TEST(!IsFullyConstant(expr));
    return 0;
}

int main() {
    BF_RUN_TEST(TestDetect_NullIsFalse);
    BF_RUN_TEST(TestDetect_ConstLeaf);
    BF_RUN_TEST(TestDetect_VarLeafFalse);
    BF_RUN_TEST(TestDetect_AllChildrenConstant);
    BF_RUN_TEST(TestDetect_AnyChildNonConstantFalse);
    return 0;
}
