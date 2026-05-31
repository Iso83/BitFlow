#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>

namespace BitFlow::Core::Rules::Simplify::Arithmetic {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_NegPowEven(const ExprStore* store, const ExprNameMap* names, ExprId id) {
    const Expr& e = (*store)[id];
    if (e.op != OpType::Pow || e.inputs.size() != 2)
        return false;

    const Expr& base = (*store)[e.inputs[0]];
    if (base.op != OpType::Neg || base.inputs.size() != 1)
        return false;

    const Expr& exp = (*store)[e.inputs[1]];
    return exp.op == OpType::Const && ((exp.knownValue & 1u) == 0u);
}

static ExprId Rewrite_NegPowEven(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    BF_CORE_ASSERT(e.op == OpType::Pow && e.inputs.size() == 2);

    const Expr& base = (*store)[e.inputs[0]];
    BF_CORE_ASSERT(base.op == OpType::Neg && base.inputs.size() == 1);

    return ctx.replace(id, store->create(OpType::Pow, {base.inputs[0], e.inputs[1]}, e.bitWidth).id);
}

Rule Get_NegPowEven_Rule() {
    return Rule{NegPowEven, &Match_NegPowEven, &Rewrite_NegPowEven};
}

} // namespace BitFlow::Core::Rules::Simplify::Arithmetic
