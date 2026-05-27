#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>
#include <utility>

namespace BitFlow::Core::Rules::Normalize {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

static bool Match_Flatten(const ExprStore* store, ExprId id) {
    const Expr& e = (*store)[id];

    if (!Expression::IsCommutative(e.op))
        return false;

    if (e.inputs.empty())
        return false;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (e.op == exprIn.op)
            return true;
    }

    return false;
}

static ExprId Rewrite_Flatten(RewriteContext& ctx, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];

    if (!Expression::IsCommutative(e.op)) {
        BF_CORE_ASSERT(false);
        return id;
    }

    ExprInputs newInputs;
    newInputs.reserve(e.inputs.size());

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (e.op == exprIn.op) {
            for (auto sub : exprIn.inputs)
                newInputs.push_back(sub);
        } else
            newInputs.push_back(in);
    }

    return ctx.replace(id, store->create(e.op, std::move(newInputs), e.bitWidth).id);
}

Rule Get_Flatten_Rule() {
    return Rule{Flatten, &Match_Flatten, &Rewrite_Flatten};
}

} // namespace BitFlow::Core::Rules::Normalize
