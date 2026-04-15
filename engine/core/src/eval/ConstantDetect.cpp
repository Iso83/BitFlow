#include <BitFlow/core/eval/ConstantDetect.h>

#include <BitFlow/core/ast/OpType.h>

namespace BitFlow::Core::Eval {

bool IsFullyConstant(const AST::Expr* root) {
    if (root == nullptr)
        return false;

    if (root->op == AST::OpType::Const)
        return true;

    if (root->op == AST::OpType::Var)
        return false;

    for (const AST::Expr* in : root->inputs) {
        if (!IsFullyConstant(in))
            return false;
    }

    return true;
}

} // namespace BitFlow::Core::Eval
