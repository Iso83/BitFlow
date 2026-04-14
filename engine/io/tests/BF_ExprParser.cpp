#include <BitFlow/io/ExprParser.h>
#include <TestAssert.h>
#include <stdexcept>

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

int TestExprParser_RejectsUnsupportedOperator() {
    bool failed = false;

    try {
        (void)BitFlow::IO::Parse("a + b");
    } catch (const std::runtime_error&) {
        failed = true;
    }

    BF_TEST(failed);
    return 0;
}

int main() {
    BF_RUN_TEST(TestExprParser_PrecedenceAndShape);
    BF_RUN_TEST(TestExprParser_RejectsUnsupportedOperator);
    return 0;
}
