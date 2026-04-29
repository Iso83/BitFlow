#include <BitFlow/core/expression/OpType.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Expression;

int TestCommutativeAssociative() {
    BF_TEST(IsCommutative(OpType::Add));
    BF_TEST(IsCommutative(OpType::Mul));
    BF_TEST(IsCommutative(OpType::And));
    BF_TEST(IsCommutative(OpType::Or));
    BF_TEST(IsCommutative(OpType::Xor));

    BF_TEST(!IsCommutative(OpType::Sub));
    BF_TEST(!IsCommutative(OpType::Div));
    BF_TEST(!IsCommutative(OpType::Mod));
    BF_TEST(!IsCommutative(OpType::Shl));
    BF_TEST(!IsCommutative(OpType::Shr));
    BF_TEST(!IsCommutative(OpType::UShr));
    BF_TEST(!IsCommutative(OpType::RotL));
    BF_TEST(!IsCommutative(OpType::RotR));

    BF_TEST(IsAssociative(OpType::Add));
    BF_TEST(IsAssociative(OpType::Mul));
    BF_TEST(IsAssociative(OpType::And));
    BF_TEST(IsAssociative(OpType::Or));
    BF_TEST(IsAssociative(OpType::Xor));

    BF_TEST(!IsAssociative(OpType::Sub));
    BF_TEST(!IsAssociative(OpType::Div));
    BF_TEST(!IsAssociative(OpType::Mod));
    BF_TEST(!IsAssociative(OpType::Shl));
    BF_TEST(!IsAssociative(OpType::Shr));
    BF_TEST(!IsAssociative(OpType::UShr));
    return 0;
}

int main() {
    BF_RUN_TEST(TestCommutativeAssociative);
    return 0;
}