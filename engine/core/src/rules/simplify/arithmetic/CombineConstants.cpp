#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
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

static bool Match_CombineConstants(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];
    if (!IsSupportedOp(e.op) || e.inputs.size() != 2)
        return false;

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];
    if (lhs.op != OpType::Const || rhs.op != OpType::Const)
        return false;

    if (e.op == OpType::Div) {
        if (rhs.knownValue == 0)
            return false;
        return (lhs.knownValue % rhs.knownValue) == 0;
    }

    return true;
}

static ExprId Rewrite_CombineConstants(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    const Expr& lhs = (*store)[e.inputs[0]];
    const Expr& rhs = (*store)[e.inputs[1]];

    switch (e.op) {
    case OpType::Add:
        return ctx.replace(
            id, store->createConstant((lhs.knownValue + rhs.knownValue) & e.fullMask(e.bitWidth), e.bitWidth).id);
    case OpType::Sub:
        if (lhs.knownValue >= rhs.knownValue)
            return ctx.replace(
                id, store->createConstant((lhs.knownValue - rhs.knownValue) & e.fullMask(e.bitWidth), e.bitWidth).id);

        return ctx.replace(
            id,
            store
                ->create(
                    OpType::Neg,
                    {store->createConstant((rhs.knownValue - lhs.knownValue) & e.fullMask(e.bitWidth), e.bitWidth).id},
                    e.bitWidth)
                .id);
    case OpType::Mul:
        return ctx.replace(
            id, store->createConstant((lhs.knownValue * rhs.knownValue) & e.fullMask(e.bitWidth), e.bitWidth).id);
    case OpType::Div:
        if (rhs.knownValue == 0) {
            BF_CORE_ASSERT(false);
            return id;
        }

        return ctx.replace(
            id, store->createConstant((lhs.knownValue / rhs.knownValue) & e.fullMask(e.bitWidth), e.bitWidth).id);
    case OpType::Mod:
        if (rhs.knownValue == 0) {
            BF_CORE_ASSERT(false);
            return id;
        }

        return ctx.replace(
            id, store->createConstant((lhs.knownValue % rhs.knownValue) & e.fullMask(e.bitWidth), e.bitWidth).id);
    default: {
        BF_CORE_ASSERT(false);
        return id;
    }
    }
}

Rule Get_CombineConstants_Rule() {
    return Rule{CombineConstants, &Match_CombineConstants, &Rewrite_CombineConstants, {Normalize::Order}};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
