#include "expression/OpTraits.h"

#include <BitFlow/core/expression/OpType.h>
#include <TestAssert.h>

using namespace BitFlow::Core::Expression;

int TestLeafAndArity() {
    BF_TEST(IsLeaf(OpType::Var));
    BF_TEST(IsLeaf(OpType::Const));
    BF_TEST(!IsLeaf(OpType::Add));

    BF_TEST(ArityOf(OpType::Var) == 0);
    BF_TEST(ArityOf(OpType::Const) == 0);
    BF_TEST(ArityOf(OpType::Neg) == 1);
    BF_TEST(ArityOf(OpType::Not) == 1);
    BF_TEST(ArityOf(OpType::Sub) == 2);
    BF_TEST(ArityOf(OpType::Div) == 2);
    BF_TEST(ArityOf(OpType::Mod) == 2);
    BF_TEST(ArityOf(OpType::Shl) == 2);
    BF_TEST(ArityOf(OpType::Shr) == 2);
    BF_TEST(ArityOf(OpType::UShr) == 2);
    BF_TEST(ArityOf(OpType::RotL) == 2);
    BF_TEST(ArityOf(OpType::RotR) == 2);
    BF_TEST(ArityOf(OpType::Ch) == 3);
    BF_TEST(ArityOf(OpType::Maj) == 3);
    BF_TEST(ArityOf(OpType::Add) == -1);
    BF_TEST(ArityOf(OpType::Mul) == -1);

    BF_TEST(HasFixedArity(OpType::RotL));
    BF_TEST(HasFixedArity(OpType::Maj));
    BF_TEST(!HasFixedArity(OpType::Add));
    BF_TEST(!HasFixedArity(OpType::Mul));
    return 0;
}

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
    BF_RUN_TEST(TestLeafAndArity);
    BF_RUN_TEST(TestCommutativeAssociative);
    return 0;
}
