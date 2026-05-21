#include <BitFlow/core/expression/ExprRefUtils.h>
#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Testing;
using namespace BitFlow::Core::Expression;

static int TestEqualChunkValue_ConstMatch() {
    MakeExprStore(32);

    auto c = C(123);

    BF_TEST(EqualChunkValue(c, 123u));
    BF_TEST(!EqualChunkValue(c, 456u));

    return 0;
}

static int TestEqualChunkValue_NonConst() {
    MakeExprStore(32);

    auto a = V("a");

    BF_TEST(!EqualChunkValue(a, 0u));
    BF_TEST(!EqualChunkValue(a, 1u));

    return 0;
}

int main() {
    BF_RUN_TEST(TestEqualChunkValue_ConstMatch);
    BF_RUN_TEST(TestEqualChunkValue_NonConst);
    return 0;
}
