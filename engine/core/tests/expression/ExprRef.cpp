#include "TestAssert.h"
#include "common/Expr.h"

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Ids;

static int TestExprRefArithmeticDsl() {
    MakeExprStore(32);

    auto a = V("a");
    auto expr = (a + 6) - 7;

    auto sub = expr;

    CPPTEST_ASSERT(Op(sub) == OpType::Sub);
    CPPTEST_ASSERT(InputSize(sub) == 2);

    auto add = Input(sub, 0);

    CPPTEST_ASSERT(Op(add) == OpType::Add);
    CPPTEST_ASSERT(InputSize(add) == 2);

    CPPTEST_ASSERT(Input(add, 0) == a);
    CPPTEST_ASSERT(EqualChunkValue(Input(add, 1), 6));

    CPPTEST_ASSERT(EqualChunkValue(Input(sub, 1), 7));

    return 0;
}

static int TestExprRefBitwiseDsl() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = V("b");

    auto expr = (~a) ^ (b & 0xff);

    auto xorExpr = expr;

    CPPTEST_ASSERT(Op(xorExpr) == OpType::Xor);
    CPPTEST_ASSERT(InputSize(xorExpr) == 2);

    auto notExpr = Input(xorExpr, 0);
    CPPTEST_ASSERT(Op(notExpr) == OpType::Not);

    auto andExpr = Input(xorExpr, 1);
    CPPTEST_ASSERT(Op(andExpr) == OpType::And);

    CPPTEST_ASSERT(EqualChunkValue(Input(andExpr, 1), 0xff));

    return 0;
}

static int TestExprRefShiftRotateDsl() {
    MakeExprStore(32);

    auto a = V("a");

    auto shl = a << 3;
    auto shr = a >> 2;
    auto rotl = a.RotL(5);
    auto rotr = a.RotR(7);

    CPPTEST_ASSERT(Op(shl) == OpType::Shl);
    CPPTEST_ASSERT(Op(shr) == OpType::Shr);
    CPPTEST_ASSERT(Op(rotl) == OpType::RotL);
    CPPTEST_ASSERT(Op(rotr) == OpType::RotR);

    CPPTEST_ASSERT(EqualChunkValue(Input(shl, 1), 3));
    CPPTEST_ASSERT(EqualChunkValue(Input(shr, 1), 2));
    CPPTEST_ASSERT(EqualChunkValue(Input(rotl, 1), 5));
    CPPTEST_ASSERT(EqualChunkValue(Input(rotr, 1), 7));

    return 0;
}

int main() {
    CPPTEST_RUN(TestExprRefArithmeticDsl);
    CPPTEST_RUN(TestExprRefBitwiseDsl);
    CPPTEST_RUN(TestExprRefShiftRotateDsl);

    return 0;
}
