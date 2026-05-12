#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

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

static bool Match_CombineConstants(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    if (!IsSupportedOp(e.op) || e.inputs.size() != 2)
        return false;

    return (*store)[e.inputs[0]].op == OpType::Const && (*store)[e.inputs[1]].op == OpType::Const;
}

static ExprId Rewrite_CombineConstants(ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[0]];

    switch (e.op) {
    case OpType::Add:
        return store->createConstant((lhs.knownValue + rhs.knownValue) & e.fullMask(e.bitWidth)).id;
    case OpType::Sub:
        return store->createConstant((lhs.knownValue - rhs.knownValue) & e.fullMask(e.bitWidth)).id;
    case OpType::Mul:
        return store->createConstant((lhs.knownValue * rhs.knownValue) & e.fullMask(e.bitWidth)).id;
    case OpType::Div:
        if (rhs.knownValue == 0) {
            _ASSERT(false);
            return id;
        }

        return store->createConstant((lhs.knownValue / rhs.knownValue) & e.fullMask(e.bitWidth)).id;
    case OpType::Mod:
        if (rhs.knownValue == 0) {
            _ASSERT(false);
            return id;
        }

        return store->createConstant((lhs.knownValue % rhs.knownValue) & e.fullMask(e.bitWidth)).id;
    default: {
        _ASSERT(false);
        return id;
    }
    }
}

Rule Get_CombineConstants_Rule() {
    return Rule{CombineConstants, &Match_CombineConstants, &Rewrite_CombineConstants, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
