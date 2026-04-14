#pragma once

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>

namespace BitFlow::Core::Rules {

inline bool IsLeaf(const AST::Expr& e) {
    return AST::IsLeaf(e.op);
}

inline bool IsNestedSameOp(const AST::Expr& parent, const AST::Expr& child) {
    return !IsLeaf(child) && child.op == parent.op;
}

template <AST::OpType Op> inline bool Match_Zero(const AST::Expr& e) {
    if (e.op != Op)
        return false;

    if (e.inputs.empty())
        return false;

    for (const AST::Expr* in : e.inputs) {
        if (in->isConst() && in->constValue == 0)
            return true;
    }

    return false;
}

} // namespace BitFlow::Core::Rules
