#include "TestAssert.h"
#include "common/Expr.h"

#include <BitFlow/engine/core/expression/ExprRefUtils.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Expression;

static int TestEqualChunkValue_ConstMatch() {
    MakeExprStore(32);

    auto c = C(123);

    CPPTEST_ASSERT(EqualChunkValue(c, 123u));
    CPPTEST_ASSERT(!EqualChunkValue(c, 456u));

    return 0;
}

static int TestEqualChunkValue_NonConst() {
    MakeExprStore(32);

    auto a = V("a");

    CPPTEST_ASSERT(!EqualChunkValue(a, 0u));
    CPPTEST_ASSERT(!EqualChunkValue(a, 1u));

    return 0;
}

int main() {
    CPPTEST_RUN(TestEqualChunkValue_ConstMatch);
    CPPTEST_RUN(TestEqualChunkValue_NonConst);
    return 0;
}
