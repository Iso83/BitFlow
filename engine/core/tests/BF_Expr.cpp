#include <BitFlow/core/expression/Expr.h>
#include <ExprTestUtils.h>
#include <TestAssert.h>
#include <iostream>
#include <stdexcept>

using namespace BitFlow::Core::Testing;
using namespace BitFlow::Core::Expression;
using namespace BitFlow::Core::Ids;

#ifdef BF_EXPR_LIFETIME_CHECKS

int Check_Input() {
    MakeExprStore(32);

    try {
        const Expr& expr = store.get(store.createConstant(10).id);

        auto tmp = store.createConstant(20);

        volatile auto size = expr.inputs.size();
        (void)size;

        BF_TEST(false && "Expected lifetime violation was not triggered");
    } catch (const std::runtime_error& e) {
        std::cout << "[OK] Lifetime violation detected:\n" << e.what() << std::endl;

        return 0;
    } catch (...) {
        BF_TEST(false && "Unexpected exception type");
    }

    return -1;
}

int Check_Field() {
    MakeExprStore(32);

    try {
        const Expr& expr = store.get(store.createConstant(10).id);

        auto tmp = store.createConstant(20);

        volatile auto value = (int64_t)expr.knownValue;
        (void)value;

        BF_TEST(false && "Expected lifetime violation was not triggered");
    } catch (const std::runtime_error& e) {
        std::cout << "[OK] Field lifetime violation detected:\n" << e.what() << std::endl;

        return 0;
    } catch (...) {
        BF_TEST(false && "Unexpected exception type");
    }

    return -1;
}

int Check_Foreach() {
    MakeExprStore(32);

    auto a = store.createVariable().id;
    auto b = store.createVariable().id;

    auto exprId = store.create(OpType::Add, {a, b}, 32).id;

    try {
        const Expr& expr = store.get(exprId);

        auto tmp = store.createConstant(20);

        for (auto in : expr.inputs) {
            (void)in;
        }

        BF_TEST(false && "Expected lifetime violation was not triggered");
    } catch (const std::runtime_error& e) {
        std::cout << "[OK] Foreach lifetime violation detected:\n" << e.what() << std::endl;

        return 0;
    } catch (...) {
        BF_TEST(false && "Unexpected exception type");
    }

    return -1;
}

#endif

int main() {
#ifdef BF_EXPR_LIFETIME_CHECKS
    BF_RUN_TEST(Check_Input);
    BF_RUN_TEST(Check_Field);
    BF_RUN_TEST(Check_Foreach);
#endif

    return 0;
}