#include <BitFlow/core/eval/ConstantEval.h>
#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Eval;

int TestEvaluate_AddSuccess() {
    auto c1 = MakeConst(1, 200);
    auto c2 = MakeConst(2, 100);
    auto expr = MakeOp(3, OpType::Add, {c1, c2});

    EvalResult r = EvaluateConstant(expr, 8);

    BF_TEST(r.status == EvalStatus::Success);
    BF_TEST(r.value == 44);
    return 0;
}

int TestEvaluate_NotConstant() {
    auto x = MakeVar(1);
    auto c = MakeConst(2, 1);
    auto expr = MakeOp(3, OpType::Add, {x, c});

    EvalResult r = EvaluateConstant(expr, 32);

    BF_TEST(r.status == EvalStatus::NotConstant);
    return 0;
}

int TestEvaluate_DivisionByZero() {
    auto a = MakeConst(1, 7);
    auto z = MakeConst(2, 0);
    auto expr = MakeOp(3, OpType::Div, {a, z});

    EvalResult r = EvaluateConstant(expr, 32);

    BF_TEST(r.status == EvalStatus::DivisionByZero);
    return 0;
}

int TestEvaluate_ModuloByZero() {
    auto a = MakeConst(1, 7);
    auto z = MakeConst(2, 0);
    auto expr = MakeOp(3, OpType::Mod, {a, z});

    EvalResult r = EvaluateConstant(expr, 32);

    BF_TEST(r.status == EvalStatus::ModuloByZero);
    return 0;
}

int TestEvaluate_InvalidBitWidth() {
    auto c = MakeConst(1, 5);

    EvalResult r0 = EvaluateConstant(c, 0);
    EvalResult r65 = EvaluateConstant(c, 65);

    BF_TEST(r0.status == EvalStatus::InvalidBitWidth);
    BF_TEST(r65.status == EvalStatus::InvalidBitWidth);
    return 0;
}

int main() {
    BF_RUN_TEST(TestEvaluate_AddSuccess);
    BF_RUN_TEST(TestEvaluate_NotConstant);
    BF_RUN_TEST(TestEvaluate_DivisionByZero);
    BF_RUN_TEST(TestEvaluate_ModuloByZero);
    BF_RUN_TEST(TestEvaluate_InvalidBitWidth);
    return 0;
}
