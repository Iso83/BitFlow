#include "expression/ExprPrinter.h"

#include <Core_Expr.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Ids;

int main() {
    MakeExprStore(32);

    auto a = V();
    auto b = (a + 6) - 7;

    auto c = a ^ b;

    std::unordered_map<ExprId, std::string> names = {{a.id, "a"}, {b.id, "b"}, {c.id, "c"}};

    std::cout << "a: " << ToString(a, names) << std::endl;
    std::cout << "b: " << ToString(b, names) << std::endl;
    std::cout << "c: " << ToString(c, names) << std::endl;

    return 0;
}
