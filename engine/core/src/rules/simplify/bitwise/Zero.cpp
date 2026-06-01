#include "expression/ExprUtils.h"

#include <BitFlow/core/rules/RewriteContext.h>
#include <BitFlow/core/rules/Rule.h>
#include <vector>

namespace BitFlow::Core::Rules::Simplify::Bitwise {

using namespace BitFlow::Core::Ids;
using namespace BitFlow::Core::Expression;

#pragma region Rewrite
static ExprId Rewrite_XorZero(RewriteContext& ctx, const ExprNameMap* names, ExprId id) {
    ExprStore* store = ctx;
    const Expr& e = (*store)[id];
    ExprInputs newInputs;

    for (auto in : e.inputs) {
        const Expr& exprIn = (*store)[in];
        if (exprIn.op == OpType::Const && store->isFalse(in))
            continue;

        newInputs.push_back(in);
    }

    if (newInputs.empty())
        return ctx.replace(id, store->makeFalse(e.bitWidth).id);

    if (newInputs.size() == 1)
        return ctx.replace(id, newInputs[0]);

    return ctx.replace(id, store->create(e.op, std::move(newInputs), e.bitWidth).id);
}
#pragma endregion

Rule Get_XorZero_Rule() {
    return Rule{XorZero, &Match_Zero<OpType::Xor>, &Rewrite_XorZero, {Normalize::Flatten}};
}

} // namespace BitFlow::Core::Rules::Simplify::Bitwise
