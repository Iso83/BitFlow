#include <BitFlow/core/eval/ConstantEval.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Eval;

int TestEvaluate_AddSubMulMasking() {
    auto a = MakeConst(1, 200);
    auto b = MakeConst(2, 100);

    auto add = MakeOp(3, OpType::Add, {a, b});
    auto sub = MakeOp(4, OpType::Sub, {a, b});
    auto mul = MakeOp(5, OpType::Mul, {a, b});

    EvalResult rAdd = EvaluateConstant(add, 8);
    EvalResult rSub = EvaluateConstant(sub, 8);
    EvalResult rMul = EvaluateConstant(mul, 8);

    BF_TEST(rAdd.status == EvalStatus::Success);
    BF_TEST(rAdd.value == 44);

    BF_TEST(rSub.status == EvalStatus::Success);
    BF_TEST(rSub.value == 100);

    BF_TEST(rMul.status == EvalStatus::Success);
    BF_TEST(rMul.value == 32);
    return 0;
}

int TestEvaluate_DivModAndGuards() {
    auto a = MakeConst(1, 20);
    auto b = MakeConst(2, 6);
    auto z = MakeConst(3, 0);

    auto div = MakeOp(4, OpType::Div, {a, b});
    auto mod = MakeOp(5, OpType::Mod, {a, b});
    auto div0 = MakeOp(6, OpType::Div, {a, z});
    auto mod0 = MakeOp(7, OpType::Mod, {a, z});

    EvalResult rDiv = EvaluateConstant(div, 8);
    EvalResult rMod = EvaluateConstant(mod, 8);
    EvalResult rDiv0 = EvaluateConstant(div0, 8);
    EvalResult rMod0 = EvaluateConstant(mod0, 8);

    BF_TEST(rDiv.status == EvalStatus::Success);
    BF_TEST(rDiv.value == 3);

    BF_TEST(rMod.status == EvalStatus::Success);
    BF_TEST(rMod.value == 2);

    BF_TEST(rDiv0.status == EvalStatus::DivisionByZero);
    BF_TEST(rMod0.status == EvalStatus::ModuloByZero);
    return 0;
}

int TestEvaluate_NegAndBitwise() {
    auto a = MakeConst(1, 0x55);
    auto b = MakeConst(2, 0x0F);

    auto neg = MakeOp(3, OpType::Neg, {a});
    auto bitAnd = MakeOp(4, OpType::And, {a, b});
    auto bitOr = MakeOp(5, OpType::Or, {a, b});
    auto bitXor = MakeOp(6, OpType::Xor, {a, b});
    auto bitNot = MakeOp(7, OpType::Not, {a});

    EvalResult rNeg = EvaluateConstant(neg, 8);
    EvalResult rAnd = EvaluateConstant(bitAnd, 8);
    EvalResult rOr = EvaluateConstant(bitOr, 8);
    EvalResult rXor = EvaluateConstant(bitXor, 8);
    EvalResult rNot = EvaluateConstant(bitNot, 8);

    BF_TEST(rNeg.status == EvalStatus::Success);
    BF_TEST(rNeg.value == 0xAB);

    BF_TEST(rAnd.status == EvalStatus::Success);
    BF_TEST(rAnd.value == 0x05);

    BF_TEST(rOr.status == EvalStatus::Success);
    BF_TEST(rOr.value == 0x5F);

    BF_TEST(rXor.status == EvalStatus::Success);
    BF_TEST(rXor.value == 0x5A);

    BF_TEST(rNot.status == EvalStatus::Success);
    BF_TEST(rNot.value == 0xAA);
    return 0;
}

int TestEvaluate_ShiftAndRotateUseModulo() {
    auto v = MakeConst(1, 0x81);
    auto n = MakeConst(2, 9);

    auto shl = MakeOp(3, OpType::Shl, {v, n});
    auto shr = MakeOp(4, OpType::Shr, {v, n});
    auto ushr = MakeOp(5, OpType::UShr, {v, n});
    auto rotl = MakeOp(6, OpType::RotL, {v, n});
    auto rotr = MakeOp(7, OpType::RotR, {v, n});

    EvalResult rShl = EvaluateConstant(shl, 8);
    EvalResult rShr = EvaluateConstant(shr, 8);
    EvalResult rUShr = EvaluateConstant(ushr, 8);
    EvalResult rRotL = EvaluateConstant(rotl, 8);
    EvalResult rRotR = EvaluateConstant(rotr, 8);

    BF_TEST(rShl.status == EvalStatus::Success);
    BF_TEST(rShl.value == 0x02);

    BF_TEST(rShr.status == EvalStatus::Success);
    BF_TEST(rShr.value == 0x40);

    BF_TEST(rUShr.status == EvalStatus::Success);
    BF_TEST(rUShr.value == 0x40);

    BF_TEST(rRotL.status == EvalStatus::Success);
    BF_TEST(rRotL.value == 0x03);

    BF_TEST(rRotR.status == EvalStatus::Success);
    BF_TEST(rRotR.value == 0xC0);
    return 0;
}

int TestEvaluate_NotConstantAndInvalidBitWidth() {
    auto x = MakeVar(1);
    auto c = MakeConst(2, 1);
    auto expr = MakeOp(3, OpType::Add, {x, c});

    EvalResult notConst = EvaluateConstant(expr, 32);
    EvalResult bw0 = EvaluateConstant(c, 0);
    EvalResult bw65 = EvaluateConstant(c, 65);

    BF_TEST(notConst.status == EvalStatus::NotConstant);
    BF_TEST(bw0.status == EvalStatus::InvalidBitWidth);
    BF_TEST(bw65.status == EvalStatus::InvalidBitWidth);
    return 0;
}

int main() {
    BF_RUN_TEST(TestEvaluate_AddSubMulMasking);
    BF_RUN_TEST(TestEvaluate_DivModAndGuards);
    BF_RUN_TEST(TestEvaluate_NegAndBitwise);
    BF_RUN_TEST(TestEvaluate_ShiftAndRotateUseModulo);
    BF_RUN_TEST(TestEvaluate_NotConstantAndInvalidBitWidth);
    return 0;
}
