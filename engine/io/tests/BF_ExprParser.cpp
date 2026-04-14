#include <BitFlow/io/ExprParser.h>
#include <TestAssert.h>

using Expr = BitFlow::Core::AST::Expr;
using OpType = BitFlow::Core::AST::OpType;

int TestExprParser_Precedence_MulBeforeAdd() {
    auto parsed = BitFlow::IO::Parse("1 + 2 * 3");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::Add);
    BF_TEST(root->inputs[0]->isConst());
    BF_TEST(root->inputs[1]->op == OpType::Mul);
    return 0;
}

int TestExprParser_Precedence_AddBeforeShift() {
    auto parsed = BitFlow::IO::Parse("1 << 2 + 3");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::Shl);
    BF_TEST(root->inputs[0]->isConst());
    BF_TEST(root->inputs[1]->op == OpType::Add);
    return 0;
}

int TestExprParser_Associativity_SubIsLeft() {
    auto parsed = BitFlow::IO::Parse("1 - 2 - 3");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::Sub);
    BF_TEST(root->inputs[0]->op == OpType::Sub);
    BF_TEST(root->inputs[1]->isConst());
    return 0;
}

int TestExprParser_ParsesUnaryAndBinaryMinus() {
    auto parsed = BitFlow::IO::Parse("-a - b");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::Sub);
    BF_TEST(root->inputs[0]->op == OpType::Neg);
    BF_TEST(root->inputs[1]->op == OpType::Var);
    return 0;
}

int TestExprParser_ParsesRotrCall() {
    auto parsed = BitFlow::IO::Parse("rotr(a, 3)");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::RotR);
    BF_TEST(root->inputs.size() == 2);
    BF_TEST(root->inputs[0]->op == OpType::Var);
    BF_TEST(root->inputs[1]->isConst());
    return 0;
}

int TestExprParser_ParsesRotlCall() {
    auto parsed = BitFlow::IO::Parse("rotl(x + y, 5)");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::RotL);
    BF_TEST(root->inputs.size() == 2);
    BF_TEST(root->inputs[0]->op == OpType::Add);
    BF_TEST(root->inputs[1]->isConst());
    return 0;
}

int TestExprParser_MixedExpressionShape() {
    auto parsed = BitFlow::IO::Parse("~a ^ b & (c + 3) << 2");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::Xor);
    BF_TEST(root->inputs[0]->op == OpType::Not);
    BF_TEST(root->inputs[1]->op == OpType::And);
    BF_TEST(root->inputs[1]->inputs[1]->op == OpType::Shl);
    BF_TEST(root->inputs[1]->inputs[1]->inputs[0]->op == OpType::Add);
    return 0;
}

int TestExprParser_ShiftOperators() {
    auto parsed = BitFlow::IO::Parse("a << b >> c >>> d");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::UShr);
    BF_TEST(root->inputs[0]->op == OpType::Shr);
    BF_TEST(root->inputs[0]->inputs[0]->op == OpType::Shl);
    return 0;
}

int main() {
    BF_RUN_TEST(TestExprParser_Precedence_MulBeforeAdd);
    BF_RUN_TEST(TestExprParser_Precedence_AddBeforeShift);
    BF_RUN_TEST(TestExprParser_Associativity_SubIsLeft);
    BF_RUN_TEST(TestExprParser_ParsesUnaryAndBinaryMinus);
    BF_RUN_TEST(TestExprParser_ParsesRotrCall);
    BF_RUN_TEST(TestExprParser_ParsesRotlCall);
    BF_RUN_TEST(TestExprParser_MixedExpressionShape);
    BF_RUN_TEST(TestExprParser_ShiftOperators);
    return 0;
}
