#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Ids;

static int TestExprRefArithmeticDsl() {
    MakeExprStore(32);

    auto a = V("a");
    auto expr = (a + 6) - 7;

    auto sub = expr;

    BF_TEST(Op(sub) == OpType::Sub);
    BF_TEST(InputSize(sub) == 2);

    auto add = Input(sub, 0);

    BF_TEST(Op(add) == OpType::Add);
    BF_TEST(InputSize(add) == 2);

    BF_TEST(Input(add, 0) == a);
    BF_TEST(EqualChunkValue(Input(add, 1), 6));

    BF_TEST(EqualChunkValue(Input(sub, 1), 7));

    return 0;
}

static int TestExprRefBitwiseDsl() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    auto expr = (~a) ^ (b & 0xff);

    auto xorExpr = expr;

    BF_TEST(Op(xorExpr) == OpType::Xor);
    BF_TEST(InputSize(xorExpr) == 2);

    auto notExpr = Input(xorExpr, 0);
    BF_TEST(Op(notExpr) == OpType::Not);

    auto andExpr = Input(xorExpr, 1);
    BF_TEST(Op(andExpr) == OpType::And);

    BF_TEST(EqualChunkValue(Input(andExpr, 1), 0xff));

    return 0;
}

static int TestExprRefShiftRotateDsl() {
    MakeExprStore(32);

    auto a = V("a");

    auto shl = a << 3;
    auto shr = a >> 2;
    auto rotl = a.RotL(5);
    auto rotr = a.RotR(7);

    BF_TEST(Op(shl) == OpType::Shl);
    BF_TEST(Op(shr) == OpType::Shr);
    BF_TEST(Op(rotl) == OpType::RotL);
    BF_TEST(Op(rotr) == OpType::RotR);

    BF_TEST(EqualChunkValue(Input(shl, 1), 3));
    BF_TEST(EqualChunkValue(Input(shr, 1), 2));
    BF_TEST(EqualChunkValue(Input(rotl, 1), 5));
    BF_TEST(EqualChunkValue(Input(rotr, 1), 7));

    return 0;
}

int main() {
    BF_RUN_TEST(TestExprRefArithmeticDsl);
    BF_RUN_TEST(TestExprRefBitwiseDsl);
    BF_RUN_TEST(TestExprRefShiftRotateDsl);

    return 0;
}