#include <BitFlow/core/ast/ExprStruct.h>
#include <BitFlow/core/rules/Rule.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Rules;

namespace {

int TestNormalizeOrder_PositiveAndNegative() {
    const Rule rule = Normalize::Get_Order_Rule();

    Expr* a = MakeVar(1);
    Expr* b = MakeVar(2);

    Expr* unsorted = MakeOp(10, OpType::Xor, {b, a});
    BF_TEST(rule.match(*unsorted));
    Expr* rewritten = rule.rewrite(*unsorted);
    Expr* expected = MakeOp(11, OpType::Xor, {a, b});
    BF_TEST(rewritten != nullptr);
    BF_TEST(BitFlow::Core::AST::StructEqual(rewritten, expected));

    Expr* sorted = MakeOp(12, OpType::Xor, {a, b});
    BF_TEST(!rule.match(*sorted));
    return 0;
}

int TestSimplifyArithmeticAddZero_PositiveAndNegative() {
    const Rule rule = Simplify::Arithmetic::Get_Add_Zero_Rule();

    Expr* a = MakeVar(20);
    Expr* withZero = MakeOp(21, OpType::Add, {a, MakeConst(22, 0)});
    BF_TEST(rule.match(*withZero));
    Expr* rewritten = rule.rewrite(*withZero);
    BF_TEST(rewritten != nullptr);
    BF_TEST(rewritten->id == a->id);

    Expr* withoutZero = MakeOp(23, OpType::Add, {a, MakeConst(24, 1)});
    BF_TEST(!rule.match(*withoutZero));
    return 0;
}

int TestSimplifyBitwiseXorCancel_PositiveAndNegative() {
    const Rule rule = Simplify::Bitwise::Get_Xor_Cancel_Rule();

    Expr* a = MakeVar(30);
    Expr* duplicate = MakeOp(31, OpType::Xor, {a, a});
    BF_TEST(rule.match(*duplicate));
    Expr* rewritten = rule.rewrite(*duplicate);
    BF_TEST(rewritten != nullptr);
    BF_TEST(rewritten->op == OpType::Const);
    BF_TEST(rewritten->constValue == 0U);

    Expr* distinct = MakeOp(32, OpType::Xor, {a, MakeVar(33)});
    BF_TEST(!rule.match(*distinct));
    return 0;
}

int TestFactorizeArithmeticLinearMultiplicity_PositiveAndNegative() {
    const Rule rule = Factorize::Arithmetic::Get_Add_LinearMultiplicity_Rule();

    Expr* a = MakeVar(40);
    Expr* linear = MakeOp(41, OpType::Add, {a, MakeOp(42, OpType::Mul, {MakeConst(43, 2), a})});
    BF_TEST(rule.match(*linear));
    Expr* rewritten = rule.rewrite(*linear);
    Expr* expected = MakeOp(44, OpType::Mul, {a, MakeConst(45, 3)});
    BF_TEST(rewritten != nullptr);
    BF_TEST(BitFlow::Core::AST::StructEqual(rewritten, expected));

    Expr* outOfScope = MakeOp(46, OpType::Add, {a, MakeOp(47, OpType::Mul, {a, MakeVar(48)})});
    BF_TEST(!rule.match(*outOfScope));
    return 0;
}

int TestFactorizeBitwiseXorAnd_PositiveAndNegative() {
    const Rule rule = Factorize::Bitwise::Get_Xor_And_Rule();

    Expr* a = MakeVar(50);
    Expr* b = MakeVar(51);
    Expr* c = MakeVar(52);
    Expr* factorable = MakeOp(53, OpType::Xor, {MakeOp(54, OpType::And, {a, b}), MakeOp(55, OpType::And, {a, c})});
    BF_TEST(rule.match(*factorable));
    Expr* rewritten = rule.rewrite(*factorable);
    Expr* expected = MakeOp(56, OpType::And, {a, MakeOp(57, OpType::Xor, {b, c})});
    BF_TEST(rewritten != nullptr);
    BF_TEST(BitFlow::Core::AST::StructEqual(rewritten, expected));

    Expr* outOfScope = MakeOp(58, OpType::Xor, {a, b});
    BF_TEST(!rule.match(*outOfScope));
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestNormalizeOrder_PositiveAndNegative);
    BF_RUN_TEST(TestSimplifyArithmeticAddZero_PositiveAndNegative);
    BF_RUN_TEST(TestSimplifyBitwiseXorCancel_PositiveAndNegative);
    BF_RUN_TEST(TestFactorizeArithmeticLinearMultiplicity_PositiveAndNegative);
    BF_RUN_TEST(TestFactorizeBitwiseXorAnd_PositiveAndNegative);
    return 0;
}
