#pragma once

#include <BitFlow/core/ast/Expression.h>
#include <BitFlow/core/ast/OpType.h>

namespace BitFlow::Core::Rules {

inline bool IsCommutative(AST::OpType op) {
    switch (op) {
    case AST::OpType::Add:
    case AST::OpType::Xor:
    case AST::OpType::And:
    case AST::OpType::Or:
        return true;
    default:
        return false;
    }
}

inline bool IsLeaf(const AST::Expr& e) {
    return e.op == AST::OpType::Var || e.op == AST::OpType::Const;
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