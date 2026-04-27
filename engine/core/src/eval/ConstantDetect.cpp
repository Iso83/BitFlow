#include <BitFlow/core/eval/ConstantDetect.h>
#include <BitFlow/core/expression/OpType.h>

namespace BitFlow::Core::Eval {

using Expr = Expression::Expr;
using OpType = Expression::OpType;

bool IsFullyConstant(const Expr* root) {
    if (root == nullptr)
        return false;

    if (root->op == OpType::Const)
        return true;

    if (root->op == OpType::Var)
        return false;

    for (const Expr* in : root->inputs) {
        if (!IsFullyConstant(in))
            return false;
    }

    return true;
}

} // namespace BitFlow::Core::Eval
