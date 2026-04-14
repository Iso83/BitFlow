#include "ast/ExprIntern.h"
#include "expression/ExprKeyBuilders.h"

#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::AST;
using namespace BitFlow::Core::Expression;

int TestStructuralKeyDifferentOrder() {
    auto a = MakeVar(1);
    auto b = MakeVar(2);

    auto e1 = MakeOp(10, OpType::Xor, {a, b});
    auto e2 = MakeOp(11, OpType::Xor, {b, a});

    auto r1 = ExprIntern::Intern(e1);
    auto r2 = ExprIntern::Intern(e2);

    BF_TEST(r1 != r2); // structureel verschillend
    return 0;
}

int TestCommutativeKeySameOrder() {
    ExprIntern::SetKeyBuilder(&BuildCommutativeKey);

    auto a = MakeVar(1);
    auto b = MakeVar(2);

    auto e1 = MakeOp(20, OpType::Xor, {a, b});
    auto e2 = MakeOp(21, OpType::Xor, {b, a});

    auto r1 = ExprIntern::Intern(e1);
    auto r2 = ExprIntern::Intern(e2);

    BF_TEST(r1 == r2); // nu gelijk

    ExprIntern::ResetKeyBuilder();
    return 0;
}

int TestConstLeafsWithSameValueDedup() {
    auto c1 = MakeConst(30, 42);
    auto c2 = MakeConst(31, 42);

    auto r1 = ExprIntern::Intern(c1);
    auto r2 = ExprIntern::Intern(c2);

    BF_TEST(r1 == r2);
    BF_TEST(r1->op == OpType::Const);
    BF_TEST(r2->op == OpType::Const);
    BF_TEST(r1->constValue == 42);
    return 0;
}

int main() {
    BF_RUN_TEST(TestStructuralKeyDifferentOrder);
    BF_RUN_TEST(TestCommutativeKeySameOrder);
    BF_RUN_TEST(TestConstLeafsWithSameValueDedup);
    return 0;
}
