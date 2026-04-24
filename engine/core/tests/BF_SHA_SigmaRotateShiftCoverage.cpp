#include <BitFlow/core/ast/ExprStruct.h>
#include <BitFlow/core/ast/OpType.h>
#include <Core_Expr.h>
#include <ProfileEngines.h>
#include <SHA_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::AST;
using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Testing::SHA;

namespace {

int CountOps(const Expr* root, OpType op) {
    if (!root)
        return 0;

    int count = (root->op == op) ? 1 : 0;
    for (const Expr* input : root->inputs)
        count += CountOps(input, op);
    return count;
}

int TestShaSafe_SmallSigmaCoverage_PreservesRotateShiftStructure() {
    Builder b;
    auto x0 = b.Var();
    auto x1 = b.Var();

    const Expr* sigma0 = MakeShaSafeEngine().ApplyUntilStable(b.SmallSigma0(x0));
    const Expr* sigma1 = MakeShaSafeEngine().ApplyUntilStable(b.SmallSigma1(x1));

    BF_TEST(CountOps(sigma0, OpType::RotR) == 2);
    BF_TEST(CountOps(sigma0, OpType::Shr) == 1);
    BF_TEST(CountOps(sigma1, OpType::RotR) == 2);
    BF_TEST(CountOps(sigma1, OpType::Shr) == 1);
    return 0;
}

int TestShaSafe_RotateModuloBitwidth_AndShiftZero_SimplifyToInput() {
    auto x = MakeVar(1001);

    auto rotrMod = MakeOp(1002, OpType::RotR, {x, MakeConst(1003, 32)}); // 32-bit rotate modulo bitwidth
    auto shrZero = MakeOp(1004, OpType::Shr, {x, MakeConst(1005, 0)});   // shift by zero

    const Expr* rewrittenRot = MakeShaSafeEngine().ApplyUntilStable(rotrMod);
    const Expr* rewrittenShr = MakeShaSafeEngine().ApplyUntilStable(shrZero);

    BF_TEST(StructEqual(rewrittenRot, x));
    BF_TEST(StructEqual(rewrittenShr, x));
    return 0;
}

} // namespace

int main() {
    BF_RUN_TEST(TestShaSafe_SmallSigmaCoverage_PreservesRotateShiftStructure);
    BF_RUN_TEST(TestShaSafe_RotateModuloBitwidth_AndShiftZero_SimplifyToInput);
    return 0;
}
