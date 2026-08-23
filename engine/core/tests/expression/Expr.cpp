#include "common/Expr.h"

#include "TestAssert.h"

#include <stdexcept>

using namespace BitFlow::Testing;
using namespace BitFlow::Engine::Core::Expression;
using namespace BitFlow::Engine::Core::Ids;

#ifdef BitFlow_EXPR_LIFETIME_CHECKS

int Check_Input() {
    MakeExprStore(32);

    try {
        const Expr& expr = store[store.createConstant(10).id];

        auto tmp = store.createConstant(20);

        volatile auto size = expr.inputs.size();
        (void)size;

        CPPTEST_ASSERT(false && "Expected lifetime violation was not triggered");
    } catch (const std::runtime_error& e) {
        std::cout << "[OK] Lifetime violation detected:\n" << e.what() << std::endl;

        return 0;
    } catch (...) {
        CPPTEST_ASSERT(false && "Unexpected exception type");
    }

    return -1;
}

int Check_Field() {
    MakeExprStore(32);

    try {
        const Expr& expr = store[store.createConstant(10).id];

        auto tmp = store.createConstant(20);

        volatile auto value = (int64_t)expr.knownValue;
        (void)value;

        CPPTEST_ASSERT(false && "Expected lifetime violation was not triggered");
    } catch (const std::runtime_error& e) {
        std::cout << "[OK] Field lifetime violation detected:\n" << e.what() << std::endl;

        return 0;
    } catch (...) {
        CPPTEST_ASSERT(false && "Unexpected exception type");
    }

    return -1;
}

int Check_Foreach() {
    MakeExprStore(32);

    auto a = store.createVariable().id;
    auto b = store.createVariable().id;

    auto exprId = store.create(OpType::Add, {a, b}, 32).id;

    try {
        const Expr& expr = store[exprId];

        auto tmp = store.createConstant(20);

        for (auto in : expr.inputs) {
            (void)in;
        }

        CPPTEST_ASSERT(false && "Expected lifetime violation was not triggered");
    } catch (const std::runtime_error& e) {
        std::cout << "[OK] Foreach lifetime violation detected:\n" << e.what() << std::endl;

        return 0;
    } catch (...) {
        CPPTEST_ASSERT(false && "Unexpected exception type");
    }

    return -1;
}

#endif

int main() {
#ifdef BitFlow_EXPR_LIFETIME_CHECKS
    CPPTEST_RUN(Check_Input);
    CPPTEST_RUN(Check_Field);
    CPPTEST_RUN(Check_Foreach);
#endif
    return 0;
}
