#include "expression/ExprPrinter.h"

#include <ExprTestUtils.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Ids;

int main() {
    MakeExprStore(32);

    auto a = V("a");
    auto b = E("b", (a + 6) - 7);
    auto c = E("c", a ^ b);

    std::cout << "a: " << ToString(a, names) << std::endl;
    std::cout << "b: " << ToString(b, names) << std::endl;
    std::cout << "c: " << ToString(c, names) << std::endl;

    return 0;
}
