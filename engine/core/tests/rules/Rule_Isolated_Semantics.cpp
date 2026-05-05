#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/rules/Rule.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Rules;

namespace {

int TestNormalizeOrder_PositiveAndNegative() {
    const Rule rule = Normalize::Get_Order_Rule();

    ExprOld* a = MakeVar(1);
    ExprOld* b = MakeVar(2);

    ExprOld* unsorted = MakeOp(10, OpType::Xor, {b, a});
    BF_TEST(rule.match(*unsorted));
    ExprOld* rewritten = rule.rewrite(*unsorted);
    ExprOld* expected = MakeOp(11, OpType::Xor, {a, b});
    BF_TEST(rewritten != nullptr);
    BF_TEST(BitFlow::Core::Expression::StructEqual(rewritten, expected));

    ExprOld* sorted = MakeOp(12, OpType::Xor, {a, b});
    BF_TEST(!rule.match(*sorted));
    return 0;
}

int TestSimplifyArithmeticAddZero_PositiveAndNegative() {
    const Rule rule = Simplify::Arithmetic::Get_Add_Zero_Rule();

    ExprOld* a = MakeVar(20);
    ExprOld* withZero = MakeOp(21, OpType::Add, {a, MakeConst(22, 0)});
    BF_TEST(rule.match(*withZero));
    ExprOld* rewritten = rule.rewrite(*withZero);
    BF_TEST(rewritten != nullptr);
    BF_TEST(rewritten->id == a->id);

    ExprOld* withoutZero = MakeOp(23, OpType::Add, {a, MakeConst(24, 1)});
    BF_TEST(!rule.match(*withoutZero));
    return 0;
}

int TestSimplifyBitwiseXorCancel_PositiveAndNegative() {
    const Rule rule = Simplify::Bitwise::Get_Xor_Cancel_Rule();

    ExprOld* a = MakeVar(30);
    ExprOld* duplicate = MakeOp(31, OpType::Xor, {a, a});
    BF_TEST(rule.match(*duplicate));
    ExprOld* rewritten = rule.rewrite(*duplicate);
    BF_TEST(rewritten != nullptr);
    BF_TEST(rewritten->op == OpType::Const);
    BF_TEST(rewritten->constValue == 0U);

    ExprOld* distinct = MakeOp(32, OpType::Xor, {a, MakeVar(33)});
    BF_TEST(!rule.match(*distinct));
    return 0;
}

int TestFactorizeArithmeticLinearMultiplicity_PositiveAndNegative() {
    const Rule rule = Factorize::Arithmetic::Get_Add_Linear_Multiplicity_Rule();

    ExprOld* a = MakeVar(40);
    ExprOld* linear = MakeOp(41, OpType::Add, {a, MakeOp(42, OpType::Mul, {MakeConst(43, 2), a})});
    BF_TEST(rule.match(*linear));
    ExprOld* rewritten = rule.rewrite(*linear);
    ExprOld* expected = MakeOp(44, OpType::Mul, {a, MakeConst(45, 3)});
    BF_TEST(rewritten != nullptr);
    BF_TEST(BitFlow::Core::Expression::StructEqual(rewritten, expected));

    ExprOld* outOfScope = MakeOp(46, OpType::Add, {a, MakeOp(47, OpType::Mul, {a, MakeVar(48)})});
    BF_TEST(!rule.match(*outOfScope));
    return 0;
}

int TestFactorizeBitwiseXorAnd_PositiveAndNegative() {
    const Rule rule = Factorize::Bitwise::Get_Xor_And_Rule();

    ExprOld* a = MakeVar(50);
    ExprOld* b = MakeVar(51);
    ExprOld* c = MakeVar(52);
    ExprOld* factorable = MakeOp(53, OpType::Xor, {MakeOp(54, OpType::And, {a, b}), MakeOp(55, OpType::And, {a, c})});
    BF_TEST(rule.match(*factorable));
    ExprOld* rewritten = rule.rewrite(*factorable);
    ExprOld* expected = MakeOp(56, OpType::And, {a, MakeOp(57, OpType::Xor, {b, c})});
    BF_TEST(rewritten != nullptr);
    BF_TEST(BitFlow::Core::Expression::StructEqual(rewritten, expected));

    ExprOld* outOfScope = MakeOp(58, OpType::Xor, {a, b});
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
