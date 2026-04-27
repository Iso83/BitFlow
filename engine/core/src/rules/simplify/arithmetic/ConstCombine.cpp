#include "rules/RuleStage.h"

#include <BitFlow/core/expression/ExprStore.h>
#include <BitFlow/core/expression/Expression.h>
#include <BitFlow/core/expression/OpType.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using Expr = Expression::Expr;
using OpType = Expression::OpType;

static bool IsSupportedOp(OpType op) {
    switch (op) {
    case OpType::Add:
    case OpType::Sub:
    case OpType::Mul:
    case OpType::Div:
    case OpType::Mod:
        return true;
    default:
        return false;
    }
}

static bool Match_Const_Combine(const Expr& e) {
    if (!IsSupportedOp(e.op) || e.inputs.size() != 2)
        return false;

    return e.inputs[0]->op == OpType::Const && e.inputs[1]->op == OpType::Const;
}

static Expr* Rewrite_Const_Combine(Expr& e) {
    const uint32_t lhs = e.inputs[0]->constValue;
    const uint32_t rhs = e.inputs[1]->constValue;

    switch (e.op) {
    case OpType::Add:
        return Expression::ConstPool::Get(lhs + rhs);
    case OpType::Sub:
        return Expression::ConstPool::Get(lhs - rhs);
    case OpType::Mul:
        return Expression::ConstPool::Get(lhs * rhs);
    case OpType::Div:
        if (rhs == 0)
            return nullptr;
        return Expression::ConstPool::Get(lhs / rhs);
    case OpType::Mod:
        if (rhs == 0)
            return nullptr;
        return Expression::ConstPool::Get(lhs % rhs);
    default:
        return nullptr;
    }
}

Rule Get_Const_Combine_Rule() {
    return Rule{RuleId::Simplify_ArithmeticConstCombine,
                &Match_Const_Combine,
                &Rewrite_Const_Combine,
                Stage_Simplify,
                {RuleId::Normalize_Flatten},
                RuleFlags::Arithmetic,
                "Simplify_ArithmeticConstCombine"};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
