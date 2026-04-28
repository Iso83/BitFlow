#include "expression/ExprPrinter.h"

#include <BitFlow/core/expression/ExprStore.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Expression;

int main() {
    ExprStore eStore;

    auto a = eStore.createVariable();
    auto b = (a + 6) - 7;

    auto c = a ^ b;

    std::unordered_map<uint32_t, std::string> names = {{a.id.value(), "a"}, {b.id.value(), "b"}, {c.id.value(), "c"}};

    std::cout << "a: " << ToString(&eStore, a.id, names) << std::endl;
    std::cout << "b: " << ToString(&eStore, b.id, names) << std::endl;
    std::cout << "c: " << ToString(&eStore, c.id, names) << std::endl;

    return 0;
}
