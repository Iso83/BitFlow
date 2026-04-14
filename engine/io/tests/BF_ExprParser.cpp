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


int TestExprParser_Associativity_SubIsLeft() {
    auto parsed = BitFlow::IO::Parse("a - b - c");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::Sub);
    BF_TEST(root->inputs[0]->op == OpType::Sub);
    BF_TEST(root->inputs[0]->inputs[0]->op == OpType::Var);
    BF_TEST(root->inputs[0]->inputs[1]->op == OpType::Var);
    BF_TEST(root->inputs[1]->op == OpType::Var);
    return 0;
}

int TestExprParser_Associativity_ShiftIsLeft() {
    auto parsed = BitFlow::IO::Parse("a << b << c");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::Shl);
    BF_TEST(root->inputs[0]->op == OpType::Shl);
    BF_TEST(root->inputs[1]->op == OpType::Var);
    return 0;
}

int TestExprParser_Associativity_BitwiseIsLeft() {
    auto parsed = BitFlow::IO::Parse("a ^ b ^ c");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::Xor);
    BF_TEST(root->inputs[0]->op == OpType::Xor);
    BF_TEST(root->inputs[1]->op == OpType::Var);
    return 0;
}


int TestExprParser_ParsesRotlCall() {
    auto parsed = BitFlow::IO::Parse("rotl(x, n)");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::RotL);
    BF_TEST(root->inputs.size() == 2);
    BF_TEST(root->inputs[0]->op == OpType::Var);
    BF_TEST(root->inputs[1]->op == OpType::Var);
    return 0;
}

int TestExprParser_ParsesRotrCallInExpression() {
    auto parsed = BitFlow::IO::Parse("rotr(x, 7) ^ y");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::Xor);
    BF_TEST(root->inputs[0]->op == OpType::RotR);
    BF_TEST(root->inputs[0]->inputs[0]->op == OpType::Var);
    BF_TEST(root->inputs[0]->inputs[1]->isConst());
    BF_TEST(root->inputs[1]->op == OpType::Var);
    return 0;
}

int TestExprParser_ParsesMulDivMod() {
    auto parsed = BitFlow::IO::Parse("a * b / c % d");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::Mod);
    BF_TEST(root->inputs[0]->op == OpType::Div);
    BF_TEST(root->inputs[0]->inputs[0]->op == OpType::Mul);
    BF_TEST(root->inputs[1]->op == OpType::Var);
    return 0;
}

int TestExprParser_ParsesAddSub() {
    auto parsed = BitFlow::IO::Parse("a + b - c");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::Sub);
    BF_TEST(root->inputs[0]->op == OpType::Add);
    BF_TEST(root->inputs[1]->op == OpType::Var);
    return 0;
}

int TestExprParser_ParsesShiftFamily() {
    auto parsed = BitFlow::IO::Parse("a << b >> c >>> d <<< e");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::UShl);
    BF_TEST(root->inputs[0]->op == OpType::UShr);
    BF_TEST(root->inputs[0]->inputs[0]->op == OpType::Shr);
    BF_TEST(root->inputs[0]->inputs[0]->inputs[0]->op == OpType::Shl);
    return 0;
}

int TestExprParser_ParsesBitwiseFamily() {
    auto parsed = BitFlow::IO::Parse("a & b ^ c | d");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::Or);
    BF_TEST(root->inputs[0]->op == OpType::Xor);
    BF_TEST(root->inputs[0]->inputs[0]->op == OpType::And);
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


int TestExprParser_ParsesNestedExpression() {
    auto parsed = BitFlow::IO::Parse("((a + (b * c)) ^ (rotl(d, 3) & ~(e)))");
    Expr* root = parsed.root;

    BF_TEST(root->op == OpType::Xor);
    BF_TEST(root->inputs[0]->op == OpType::Add);
    BF_TEST(root->inputs[0]->inputs[1]->op == OpType::Mul);
    BF_TEST(root->inputs[1]->op == OpType::And);
    BF_TEST(root->inputs[1]->inputs[0]->op == OpType::RotL);
    BF_TEST(root->inputs[1]->inputs[1]->op == OpType::Not);
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
    BF_RUN_TEST(TestExprParser_Associativity_SubIsLeft);
    BF_RUN_TEST(TestExprParser_Associativity_ShiftIsLeft);
    BF_RUN_TEST(TestExprParser_Associativity_BitwiseIsLeft);
    BF_RUN_TEST(TestExprParser_ParsesRotlCall);
    BF_RUN_TEST(TestExprParser_ParsesRotrCallInExpression);
    BF_RUN_TEST(TestExprParser_ParsesMulDivMod);
    BF_RUN_TEST(TestExprParser_ParsesAddSub);
    BF_RUN_TEST(TestExprParser_ParsesShiftFamily);
    BF_RUN_TEST(TestExprParser_ParsesBitwiseFamily);
    BF_RUN_TEST(TestExprParser_ParsesMulAddShiftBitwiseChain);
    BF_RUN_TEST(TestExprParser_ParsesNestedExpression);
    BF_RUN_TEST(TestExprParser_ParsesUnsignedShifts);
    return 0;
}
