#include <BitFlow/io/ExprParser.h>
#include <TestAssert.h>

using Expr = BitFlow::Core::AST::Expr;
using OpType = BitFlow::Core::AST::OpType;

int TestExprParser_PrecedenceAndShape() {
    auto parsed = BitFlow::IO::Parse("a ^ b & c");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::Xor);
    BF_TEST(root->inputs.size() == 2);
    BF_TEST(root->inputs[0]->op == OpType::Var);
    BF_TEST(root->inputs[1]->op == OpType::And);
    BF_TEST(root->inputs[1]->inputs.size() == 2);
    return 0;
}

int TestExprParser_ParsesUnaryAndBinaryMinus() {
    auto parsed = BitFlow::IO::Parse("-a - b");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::Sub);
    BF_TEST(root->inputs.size() == 2);
    BF_TEST(root->inputs[0]->op == OpType::Neg);
    BF_TEST(root->inputs[0]->inputs.size() == 1);
    BF_TEST(root->inputs[0]->inputs[0]->op == OpType::Var);
    BF_TEST(root->inputs[1]->op == OpType::Var);
    return 0;
}

int TestExprParser_ParsesMulAddShiftBitwiseChain() {
    auto parsed = BitFlow::IO::Parse("a + b * c << d | e");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::Or);
    BF_TEST(root->inputs[0]->op == OpType::Shl);
    BF_TEST(root->inputs[0]->inputs[0]->op == OpType::Add);
    BF_TEST(root->inputs[0]->inputs[0]->inputs[1]->op == OpType::Mul);
    BF_TEST(root->inputs[1]->op == OpType::Var);
    return 0;
}

int TestExprParser_ParsesUnsignedShifts() {
    auto parsed = BitFlow::IO::Parse("a >>> b <<< c");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::UShl);
    BF_TEST(root->inputs[0]->op == OpType::UShr);
    BF_TEST(root->inputs[1]->op == OpType::Var);
    return 0;
}

int main() {
    BF_RUN_TEST(TestExprParser_PrecedenceAndShape);
    BF_RUN_TEST(TestExprParser_ParsesUnaryAndBinaryMinus);
    BF_RUN_TEST(TestExprParser_ParsesMulAddShiftBitwiseChain);
    BF_RUN_TEST(TestExprParser_ParsesUnsignedShifts);
    return 0;
}
