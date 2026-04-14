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

int TestCommutativeKeySameOrder_AddMul() {
    ExprIntern::SetKeyBuilder(&BuildCommutativeKey);

    auto a = MakeVar(40);
    auto b = MakeVar(41);

    auto add1 = MakeOp(42, OpType::Add, {a, b});
    auto add2 = MakeOp(43, OpType::Add, {b, a});
    auto mul1 = MakeOp(44, OpType::Mul, {a, b});
    auto mul2 = MakeOp(45, OpType::Mul, {b, a});

    auto addR1 = ExprIntern::Intern(add1);
    auto addR2 = ExprIntern::Intern(add2);
    auto mulR1 = ExprIntern::Intern(mul1);
    auto mulR2 = ExprIntern::Intern(mul2);

    BF_TEST(addR1 == addR2);
    BF_TEST(mulR1 == mulR2);

    ExprIntern::ResetKeyBuilder();
    return 0;
}

int TestNonCommutativeKeyDifferentOrder_NewFamilies() {
    ExprIntern::SetKeyBuilder(&BuildCommutativeKey);

    auto a = MakeVar(50);
    auto b = MakeVar(51);

    auto sub1 = MakeOp(52, OpType::Sub, {a, b});
    auto sub2 = MakeOp(53, OpType::Sub, {b, a});
    auto shl1 = MakeOp(54, OpType::Shl, {a, b});
    auto shl2 = MakeOp(55, OpType::Shl, {b, a});
    auto rotl1 = MakeOp(56, OpType::RotL, {a, b});
    auto rotl2 = MakeOp(57, OpType::RotL, {b, a});

    auto subR1 = ExprIntern::Intern(sub1);
    auto subR2 = ExprIntern::Intern(sub2);
    auto shlR1 = ExprIntern::Intern(shl1);
    auto shlR2 = ExprIntern::Intern(shl2);
    auto rotlR1 = ExprIntern::Intern(rotl1);
    auto rotlR2 = ExprIntern::Intern(rotl2);

    BF_TEST(subR1 != subR2);
    BF_TEST(shlR1 != shlR2);
    BF_TEST(rotlR1 != rotlR2);

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
    BF_RUN_TEST(TestCommutativeKeySameOrder_AddMul);
    BF_RUN_TEST(TestNonCommutativeKeyDifferentOrder_NewFamilies);
    BF_RUN_TEST(TestConstLeafsWithSameValueDedup);
    return 0;
}
