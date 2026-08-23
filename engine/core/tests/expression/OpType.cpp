#include "TestAssert.h"

#include <BitFlow/engine/core/expression/OpType.h>

using namespace BitFlow::Engine::Core::Expression;

int TestCommutativeAssociative() {
    CPPTEST_ASSERT(IsCommutative(OpType::Add));
    CPPTEST_ASSERT(IsCommutative(OpType::Mul));
    CPPTEST_ASSERT(IsCommutative(OpType::And));
    CPPTEST_ASSERT(IsCommutative(OpType::Or));
    CPPTEST_ASSERT(IsCommutative(OpType::Xor));

    CPPTEST_ASSERT(!IsCommutative(OpType::Sub));
    CPPTEST_ASSERT(!IsCommutative(OpType::Div));
    CPPTEST_ASSERT(!IsCommutative(OpType::Mod));
    CPPTEST_ASSERT(!IsCommutative(OpType::Shl));
    CPPTEST_ASSERT(!IsCommutative(OpType::Shr));
    CPPTEST_ASSERT(!IsCommutative(OpType::RotL));
    CPPTEST_ASSERT(!IsCommutative(OpType::RotR));

    CPPTEST_ASSERT(IsAssociative(OpType::Add));
    CPPTEST_ASSERT(IsAssociative(OpType::Mul));
    CPPTEST_ASSERT(IsAssociative(OpType::And));
    CPPTEST_ASSERT(IsAssociative(OpType::Or));
    CPPTEST_ASSERT(IsAssociative(OpType::Xor));

    CPPTEST_ASSERT(!IsAssociative(OpType::Sub));
    CPPTEST_ASSERT(!IsAssociative(OpType::Div));
    CPPTEST_ASSERT(!IsAssociative(OpType::Mod));
    CPPTEST_ASSERT(!IsAssociative(OpType::Shl));
    CPPTEST_ASSERT(!IsAssociative(OpType::Shr));
    return 0;
}

int main() {
    CPPTEST_RUN(TestCommutativeAssociative);
    return 0;
}
