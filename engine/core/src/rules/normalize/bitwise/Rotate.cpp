#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Normalize::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool IsConstFalse(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];
    return e.op == OpType::Const && e.inputs.empty() && store->isFalse(id);
}

static bool Match_RotateModulo(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (e.op != OpType::RotL && e.op != OpType::RotR)
        return false;

    if (e.inputs.size() != 2)
        return false;

    const Expr& amount = (*store)[e.inputs[1]];
    if (!(amount.op == OpType::Const && amount.inputs.empty()))
        return false;

    return e.bitWidth > 0 && amount.knownValue >= e.bitWidth;
}

static ExprId Rewrite_RotateModulo(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    const Expr& amount = (*store)[e.inputs[1]];

    const Types::ExprChunk reduced = amount.knownValue % e.bitWidth;

    if (reduced == 0)
        return ctx.replace(id, e.inputs[0]);

    const OpType op = e.op;
    const ExprId in0 = e.inputs[0];
    const Types::BitWidth bitWidth = e.bitWidth;
    return ctx.replace(id, store->create(op, {in0, store->createConstant(reduced, bitWidth).id}, bitWidth).id);
}

Rule Get_RotateModulo_Rule() {
    return Rule{RotateModulo, &Match_RotateModulo, &Rewrite_RotateModulo};
}

} // namespace BitFlow::Core::Rules::Normalize::Bitwise