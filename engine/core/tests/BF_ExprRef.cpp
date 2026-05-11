#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Ids;

static int TestExprRefArithmeticDsl() {
    MakeExprStore(32);

    auto a = V("a");
    auto expr = (a + 6) - 7;

    const Expr& sub = store[expr.id];

    BF_TEST(sub.op == OpType::Sub);
    BF_TEST(sub.inputs.size() == 2);

    const Expr& add = store[sub.inputs[0]];

    BF_TEST(add.op == OpType::Add);
    BF_TEST(add.inputs.size() == 2);

    BF_TEST(ERef(add.inputs[0]) == a);
    BF_TEST(EqualChunkValue(ERef(add.inputs[1]), 6));

    BF_TEST(EqualChunkValue(ERef(sub.inputs[1]), 7));

    return 0;
}

static int TestExprRefBitwiseDsl() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    auto expr = (~a) ^ (b & 0xff);

    const Expr& xorExpr = store[expr.id];

    BF_TEST(xorExpr.op == OpType::Xor);
    BF_TEST(xorExpr.inputs.size() == 2);

    const Expr& notExpr = store[xorExpr.inputs[0]];
    BF_TEST(notExpr.op == OpType::Not);

    const Expr& andExpr = store[xorExpr.inputs[1]];
    BF_TEST(andExpr.op == OpType::And);

    BF_TEST(EqualChunkValue(ERef(andExpr.inputs[1]), 0xff));

    return 0;
}

static int TestExprRefShiftRotateDsl() {
    MakeExprStore(32);

    auto a = V("a");

    auto shl = a << 3;
    auto shr = a >> 2;
    auto rotl = a.RotL(5);
    auto rotr = a.RotR(7);

    BF_TEST(store[shl.id].op == OpType::Shl);
    BF_TEST(store[shr.id].op == OpType::Shr);
    BF_TEST(store[rotl.id].op == OpType::RotL);
    BF_TEST(store[rotr.id].op == OpType::RotR);

    BF_TEST(EqualChunkValue(ERef(store[shl.id].inputs[1]), 3));
    BF_TEST(EqualChunkValue(ERef(store[shr.id].inputs[1]), 2));
    BF_TEST(EqualChunkValue(ERef(store[rotl.id].inputs[1]), 5));
    BF_TEST(EqualChunkValue(ERef(store[rotr.id].inputs[1]), 7));

    return 0;
}

int main() {
    BF_RUN_TEST(TestExprRefArithmeticDsl);
    BF_RUN_TEST(TestExprRefBitwiseDsl);
    BF_RUN_TEST(TestExprRefShiftRotateDsl);

    return 0;
}